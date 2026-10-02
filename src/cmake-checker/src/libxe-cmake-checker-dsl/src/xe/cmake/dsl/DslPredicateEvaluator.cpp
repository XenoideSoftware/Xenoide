#include "DslPredicateEvaluator.h"

#include "xe/cmake/core/QueryPrimitives.h"

#include <cctype>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stack>
#include <utility>
#include <variant>
#include <vector>

namespace xe::cmake::dsl {

    using xe::cmake::core::StringPrimitives;

    namespace {

        // ---------------------------------------------------------------------------
        // Expression AST
        // ---------------------------------------------------------------------------

        struct Expr;

        using ExprPtr = std::shared_ptr<Expr>;

        struct Expr {
            virtual ~Expr() = default;
        };

        struct LiteralExpr : Expr {
            DslValue value;
        };

        struct IdentExpr : Expr {
            std::string name;
        };

        struct LambdaExpr : Expr {
            std::string parameter;
            ExprPtr body;
        };

        struct ListExpr : Expr {
            std::vector<ExprPtr> elements;
        };

        struct CallExpr : Expr {
            std::string callee;
            std::vector<ExprPtr> args;
        };

        struct PropertyAccessExpr : Expr {
            ExprPtr object;
            std::string property;
            std::vector<ExprPtr> args; // non-empty => method call
        };

        struct UnaryExpr : Expr {
            char op = 0;
            ExprPtr operand;
        };

        enum class BinaryOp : uint8_t {
            Eq,
            Ne,
            Lt,
            Le,
            Gt,
            Ge,
            And,
            Or,
            In,
            NotIn,
        };

        struct BinaryExpr : Expr {
            BinaryOp op = BinaryOp::Eq;
            ExprPtr left;
            ExprPtr right;
        };

        // ---------------------------------------------------------------------------
        // Tokenizer
        // ---------------------------------------------------------------------------

        enum class TokenKind : uint8_t {
            Ident,
            String,
            Number,
            Arrow,
            LParen,
            RParen,
            LBracket,
            RBracket,
            Comma,
            Eq,
            Ne,
            Lt,
            Le,
            Gt,
            Ge,
            And,
            Or,
            Not,
            In,
            Dot,
            End,
        };

        struct Token {
            TokenKind kind = TokenKind::End;
            std::string text;
        };

        class Lexer {
        public:
            explicit Lexer(std::string_view source) : source_(source) {
            }

            std::vector<Token> tokenize() {
                std::vector<Token> tokens;
                while (pos_ < source_.size()) {
                    const char c = source_[pos_];
                    if (std::isspace(static_cast<unsigned char>(c))) {
                        ++pos_;
                        continue;
                    }
                    if (c == '\'' || c == '"') {
                        tokens.push_back(lex_string(c));
                        continue;
                    }
                    if (std::isdigit(static_cast<unsigned char>(c))) {
                        tokens.push_back(lex_number());
                        continue;
                    }
                    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                        tokens.push_back(lex_identifier());
                        continue;
                    }
                    if (c == '-' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '>') {
                        pos_ += 2;
                        tokens.push_back(Token{TokenKind::Arrow, "->"});
                        continue;
                    }
                    if (c == '-' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '-') {
                        while (pos_ < source_.size() && source_[pos_] != '\n') {
                            ++pos_;
                        }
                        continue;
                    }
                    switch (c) {
                    case '(':
                        pos_ += 1;
                        tokens.push_back(Token{TokenKind::LParen, "("});
                        break;
                    case ')':
                        pos_ += 1;
                        tokens.push_back(Token{TokenKind::RParen, ")"});
                        break;
                    case '[':
                        pos_ += 1;
                        tokens.push_back(Token{TokenKind::LBracket, "["});
                        break;
                    case ']':
                        pos_ += 1;
                        tokens.push_back(Token{TokenKind::RBracket, "]"});
                        break;
                    case ',':
                        pos_ += 1;
                        tokens.push_back(Token{TokenKind::Comma, ","});
                        break;
                    case '.':
                        pos_ += 1;
                        tokens.push_back(Token{TokenKind::Dot, "."});
                        break;
                    case '!': {
                        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
                            pos_ += 2;
                            tokens.push_back(Token{TokenKind::Ne, "!="});
                        } else {
                            pos_ += 1;
                            tokens.push_back(Token{TokenKind::Not, "!"});
                        }
                        break;
                    }
                    case '=': {
                        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
                            pos_ += 2;
                            tokens.push_back(Token{TokenKind::Eq, "=="});
                        } else {
                            ++pos_;
                        }
                        break;
                    }
                    case '<': {
                        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
                            pos_ += 2;
                            tokens.push_back(Token{TokenKind::Le, "<="});
                        } else {
                            pos_ += 1;
                            tokens.push_back(Token{TokenKind::Lt, "<"});
                        }
                        break;
                    }
                    case '>': {
                        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
                            pos_ += 2;
                            tokens.push_back(Token{TokenKind::Ge, ">="});
                        } else {
                            pos_ += 1;
                            tokens.push_back(Token{TokenKind::Gt, ">"});
                        }
                        break;
                    }
                    case '&': {
                        pos_ += 2;
                        tokens.push_back(Token{TokenKind::And, "&&"});
                        break;
                    }
                    case '|': {
                        pos_ += 2;
                        tokens.push_back(Token{TokenKind::Or, "||"});
                        break;
                    }
                    default:
                        ++pos_;
                        break;
                    }
                }
                tokens.push_back(Token{TokenKind::End, ""});
                return tokens;
            }

        private:
            Token lex_string(char quote) {
                ++pos_;
                std::string text;
                while (pos_ < source_.size() && source_[pos_] != quote) {
                    if (source_[pos_] == '\\' && pos_ + 1 < source_.size()) {
                        ++pos_;
                        text.push_back(source_[pos_]);
                    } else {
                        text.push_back(source_[pos_]);
                    }
                    ++pos_;
                }
                if (pos_ < source_.size()) {
                    ++pos_;
                }
                return Token{TokenKind::String, text};
            }

            Token lex_number() {
                std::string text;
                while (pos_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[pos_]))) {
                    text.push_back(source_[pos_]);
                    ++pos_;
                }
                return Token{TokenKind::Number, text};
            }

            Token lex_identifier() {
                std::string text;
                while (pos_ < source_.size() && (std::isalnum(static_cast<unsigned char>(source_[pos_])) || source_[pos_] == '_')) {
                    text.push_back(source_[pos_]);
                    ++pos_;
                }
                if (text == "in") {
                    return Token{TokenKind::In, text};
                }
                if (text == "not") {
                    return Token{TokenKind::Not, text};
                }
                return Token{TokenKind::Ident, text};
            }

            std::string_view source_;
            std::size_t pos_ = 0;
        };

        // ---------------------------------------------------------------------------
        // Parser (recursive descent)
        // ---------------------------------------------------------------------------

        class Parser {
        public:
            explicit Parser(const std::vector<Token> &tokens) : tokens_(tokens) {
            }

            ExprPtr parse() {
                ExprPtr result = parse_or();
                if (current().kind != TokenKind::End) {
                    return nullptr;
                }
                return result;
            }

        private:
            const Token &peek() const {
                return tokens_[pos_];
            }

            const Token &next() {
                const Token &token = tokens_[pos_];
                if (pos_ + 1 < tokens_.size()) {
                    ++pos_;
                }
                return token;
            }

            bool accept(TokenKind kind) {
                if (current().kind == kind) {
                    ++pos_;
                    return true;
                }
                return false;
            }

            bool expect(TokenKind kind) {
                if (current().kind == kind) {
                    ++pos_;
                    return true;
                }
                return false;
            }

            const Token &current() const {
                return tokens_[pos_];
            }

            ExprPtr parse_or() {
                ExprPtr left = parse_and();
                while (current().kind == TokenKind::Or) {
                    next();
                    ExprPtr right = parse_and();
                    auto expr = std::make_shared<BinaryExpr>();
                    expr->op = BinaryOp::Or;
                    expr->left = left;
                    expr->right = right;
                    left = expr;
                }
                return left;
            }

            ExprPtr parse_and() {
                ExprPtr left = parse_membership();
                while (current().kind == TokenKind::And) {
                    next();
                    ExprPtr right = parse_membership();
                    auto expr = std::make_shared<BinaryExpr>();
                    expr->op = BinaryOp::And;
                    expr->left = left;
                    expr->right = right;
                    left = expr;
                }
                return left;
            }

            ExprPtr parse_membership() {
                ExprPtr left = parse_equality();
                if (current().kind == TokenKind::In) {
                    next();
                    ExprPtr right = parse_equality();
                    auto expr = std::make_shared<BinaryExpr>();
                    expr->op = BinaryOp::In;
                    expr->left = left;
                    expr->right = right;
                    return expr;
                }
                if (current().kind == TokenKind::Not && pos_ + 1 < tokens_.size() && tokens_[pos_ + 1].kind == TokenKind::In) {
                    next();
                    next();
                    ExprPtr right = parse_equality();
                    auto expr = std::make_shared<BinaryExpr>();
                    expr->op = BinaryOp::NotIn;
                    expr->left = left;
                    expr->right = right;
                    return expr;
                }
                return left;
            }

            ExprPtr parse_equality() {
                ExprPtr left = parse_relational();
                while (current().kind == TokenKind::Eq || current().kind == TokenKind::Ne) {
                    const TokenKind kind = current().kind;
                    next();
                    ExprPtr right = parse_relational();
                    auto expr = std::make_shared<BinaryExpr>();
                    expr->op = (kind == TokenKind::Eq) ? BinaryOp::Eq : BinaryOp::Ne;
                    expr->left = left;
                    expr->right = right;
                    left = expr;
                }
                return left;
            }

            ExprPtr parse_relational() {
                ExprPtr left = parse_unary();
                while (current().kind == TokenKind::Lt || current().kind == TokenKind::Le || current().kind == TokenKind::Gt || current().kind == TokenKind::Ge) {
                    const TokenKind kind = current().kind;
                    next();
                    ExprPtr right = parse_unary();
                    auto expr = std::make_shared<BinaryExpr>();
                    switch (kind) {
                    case TokenKind::Lt:
                        expr->op = BinaryOp::Lt;
                        break;
                    case TokenKind::Le:
                        expr->op = BinaryOp::Le;
                        break;
                    case TokenKind::Gt:
                        expr->op = BinaryOp::Gt;
                        break;
                    case TokenKind::Ge:
                        expr->op = BinaryOp::Ge;
                        break;
                    default:
                        break;
                    }
                    expr->left = left;
                    expr->right = right;
                    left = expr;
                }
                return left;
            }

            ExprPtr parse_unary() {
                if (current().kind == TokenKind::Not) {
                    next();
                    auto expr = std::make_shared<UnaryExpr>();
                    expr->op = '!';
                    expr->operand = parse_unary();
                    return expr;
                }
                return parse_postfix();
            }

            ExprPtr parse_postfix() {
                ExprPtr expr = parse_primary();
                while (current().kind == TokenKind::Dot) {
                    next();
                    if (current().kind != TokenKind::Ident) {
                        return expr;
                    }
                    const std::string property = next().text;
                    auto access = std::make_shared<PropertyAccessExpr>();
                    access->object = expr;
                    access->property = property;
                    if (accept(TokenKind::LParen)) {
                        while (current().kind != TokenKind::RParen) {
                            access->args.push_back(parse_or());
                            if (!accept(TokenKind::Comma)) {
                                break;
                            }
                        }
                        expect(TokenKind::RParen);
                    }
                    expr = access;
                }
                return expr;
            }

            ExprPtr parse_primary() {
                if (current().kind == TokenKind::LParen) {
                    next();
                    ExprPtr expr = parse_or();
                    expect(TokenKind::RParen);
                    return expr;
                }
                if (current().kind == TokenKind::LBracket) {
                    next();
                    auto list = std::make_shared<ListExpr>();
                    while (current().kind != TokenKind::RBracket && current().kind != TokenKind::End) {
                        list->elements.push_back(parse_or());
                        if (!accept(TokenKind::Comma)) {
                            break;
                        }
                    }
                    expect(TokenKind::RBracket);
                    return list;
                }
                if (current().kind == TokenKind::String) {
                    auto literal = std::make_shared<LiteralExpr>();
                    literal->value = DslValue::make_string(next().text);
                    return literal;
                }
                if (current().kind == TokenKind::Number) {
                    auto literal = std::make_shared<LiteralExpr>();
                    literal->value = DslValue::make_int(std::stoll(next().text));
                    return literal;
                }
                if (current().kind == TokenKind::Ident) {
                    const std::string name = next().text;
                    if (name == "true") {
                        auto literal = std::make_shared<LiteralExpr>();
                        literal->value = DslValue::make_bool(true);
                        return literal;
                    }
                    if (name == "false") {
                        auto literal = std::make_shared<LiteralExpr>();
                        literal->value = DslValue::make_bool(false);
                        return literal;
                    }
                    if (accept(TokenKind::LParen)) {
                        auto call = std::make_shared<CallExpr>();
                        call->callee = name;
                        while (current().kind != TokenKind::RParen && current().kind != TokenKind::End) {
                            call->args.push_back(parse_or());
                            if (!accept(TokenKind::Comma)) {
                                break;
                            }
                        }
                        expect(TokenKind::RParen);
                        return call;
                    }
                    // Lambda: identifier followed by -> (with optional parentheses).
                    if (accept(TokenKind::Arrow)) {
                        auto lambda = std::make_shared<LambdaExpr>();
                        lambda->parameter = name;
                        lambda->body = parse_or();
                        return lambda;
                    }
                    auto ident = std::make_shared<IdentExpr>();
                    ident->name = name;
                    return ident;
                }
                return nullptr;
            }

            const std::vector<Token> &tokens_;
            std::size_t pos_ = 0;
        };

        // ---------------------------------------------------------------------------
        // Evaluator
        // ---------------------------------------------------------------------------

        class Evaluator {
        public:
            explicit Evaluator(const DslContext &context) {
                bind("file", make_file_value(*context.file));
                bind("cmd", make_command_value(*context.command));
                bind("arg", make_argument_value(*context.argument));
                bind("span", make_span_value(context.command != nullptr ? context.command->span : xe::cmake::core::SourceSpan::invalid()));
            }

            std::optional<DslValue> evaluate(const ExprPtr &expr) {
                const LiteralExpr *literal = dynamic_cast<LiteralExpr *>(expr.get());
                if (literal != nullptr) {
                    return literal->value;
                }

                const IdentExpr *ident = dynamic_cast<IdentExpr *>(expr.get());
                if (ident != nullptr) {
                    if (ident->name == "null") {
                        return DslValue::null();
                    }
                    const auto it = scope_.find(ident->name);
                    if (it != scope_.end()) {
                        return it->second;
                    }
                    return std::nullopt;
                }

                const LambdaExpr *lambda = dynamic_cast<LambdaExpr *>(expr.get());
                if (lambda != nullptr) {
                    // A lambda evaluates to a sentinel node value carrying its body.
                    DslValue value = DslValue::null();
                    value.node_value.kind = DslNodeRef::Kind::File;
                    value.string_value = "$lambda";
                    return value;
                }

                const ListExpr *list = dynamic_cast<ListExpr *>(expr.get());
                if (list != nullptr) {
                    std::vector<DslValue> elements;
                    for (const ExprPtr &element : list->elements) {
                        std::optional<DslValue> value = evaluate(element);
                        if (!value.has_value()) {
                            return std::nullopt;
                        }
                        elements.push_back(std::move(*value));
                    }
                    return DslValue::make_list(std::move(elements));
                }

                const CallExpr *call = dynamic_cast<CallExpr *>(expr.get());
                if (call != nullptr) {
                    return evaluate_call(call->callee, call->args);
                }

                const PropertyAccessExpr *access = dynamic_cast<PropertyAccessExpr *>(expr.get());
                if (access != nullptr) {
                    std::optional<DslValue> object = evaluate(access->object);
                    if (!object.has_value()) {
                        return std::nullopt;
                    }
                    if (!access->args.empty()) {
                        return evaluate_method(*object, access->property, access->args);
                    }
                    return evaluate_property(*object, access->property);
                }

                const UnaryExpr *unary = dynamic_cast<UnaryExpr *>(expr.get());
                if (unary != nullptr) {
                    std::optional<DslValue> operand = evaluate(unary->operand);
                    if (unary->op == '!') {
                        // A missing property is falsy, so !missing is true.
                        if (!operand.has_value()) {
                            return DslValue::make_bool(true);
                        }
                        return DslValue::make_bool(!operand->is_truthy());
                    }
                    return std::nullopt;
                }

                const BinaryExpr *binary = dynamic_cast<BinaryExpr *>(expr.get());
                if (binary != nullptr) {
                    return evaluate_binary(binary);
                }

                return std::nullopt;
            }

            void bind(std::string name, DslValue value) {
                scope_[std::move(name)] = std::move(value);
            }

        private:
            static DslValue make_file_value(const xe::cmake::core::ConcreteSyntaxTree &file) {
                DslNodeRef ref;
                ref.kind = DslNodeRef::Kind::File;
                ref.file = &file;
                return DslValue::make_node(ref);
            }

            static DslValue make_command_value(const xe::cmake::core::CommandNode &command) {
                DslNodeRef ref;
                ref.kind = DslNodeRef::Kind::Command;
                ref.command = &command;
                return DslValue::make_node(ref);
            }

            static DslValue make_argument_value(const xe::cmake::core::ArgumentNode &argument) {
                DslNodeRef ref;
                ref.kind = DslNodeRef::Kind::Argument;
                ref.argument = &argument;
                return DslValue::make_node(ref);
            }

            static DslValue make_span_value(xe::cmake::core::SourceSpan span) {
                DslNodeRef ref;
                ref.kind = DslNodeRef::Kind::Span;
                ref.span = span;
                return DslValue::make_node(ref);
            }

            std::vector<DslValue> commands_of_file(const DslValue &file_value) {
                std::vector<DslValue> commands;
                if (file_value.node_value.file == nullptr) {
                    return commands;
                }
                for (const xe::cmake::core::CommandNode *command : file_value.node_value.file->commands()) {
                    commands.push_back(make_command_value(*command));
                }
                return commands;
            }

            std::vector<DslValue> arguments_of_command(const DslValue &command_value) {
                std::vector<DslValue> arguments;
                if (command_value.node_value.command == nullptr) {
                    return arguments;
                }
                const xe::cmake::core::CommandNode &command = *command_value.node_value.command;
                for (std::size_t i = 0; i < command.arguments.size(); ++i) {
                    arguments.push_back(make_argument_value(command.arguments[i]));
                }
                return arguments;
            }

            std::optional<DslValue> evaluate_property(const DslValue &object, const std::string &property) {
                if (object.kind != DslValue::Kind::Node) {
                    if (object.kind == DslValue::Kind::List) {
                        if (property == "size") {
                            return DslValue::make_int(static_cast<long long>(object.list_value.size()));
                        }
                    }
                    if (object.kind == DslValue::Kind::String) {
                        if (property == "length") {
                            return DslValue::make_int(static_cast<long long>(object.string_value.size()));
                        }
                    }
                    return std::nullopt;
                }

                const DslNodeRef &ref = object.node_value;
                if (ref.kind == DslNodeRef::Kind::File && ref.file != nullptr) {
                    if (property == "path") {
                        return DslValue::make_string(ref.file->file_path());
                    }
                    if (property == "directory") {
                        const std::string path = ref.file->file_path();
                        const std::size_t slash = path.find_last_of('/');
                        if (slash != std::string::npos) {
                            return DslValue::make_string(path.substr(0, slash));
                        }
                        return DslValue::make_string("");
                    }
                    if (property == "folder_name") {
                        const std::string path = ref.file->file_path();
                        const std::size_t slash = path.find_last_of('/');
                        if (slash == std::string::npos || slash == 0) {
                            return DslValue::make_string("");
                        }
                        const std::string parent = path.substr(0, slash);
                        const std::size_t parent_slash = parent.find_last_of('/');
                        if (parent_slash != std::string::npos && parent_slash + 1 < parent.size()) {
                            return DslValue::make_string(parent.substr(parent_slash + 1));
                        }
                        return DslValue::make_string(parent);
                    }
                    if (property == "commands") {
                        return DslValue::make_list(commands_of_file(object));
                    }
                }
                if (ref.kind == DslNodeRef::Kind::Command && ref.command != nullptr) {
                    const xe::cmake::core::CommandNode &command = *ref.command;
                    if (property == "name") {
                        return DslValue::make_string(command.name);
                    }
                    if (property == "arguments") {
                        return DslValue::make_list(arguments_of_command(object));
                    }
                    if (property == "argument_count") {
                        return DslValue::make_int(static_cast<long long>(command.argument_count()));
                    }
                    if (property == "span") {
                        return make_span_value(command.span);
                    }
                    if (property == "file_path") {
                        return DslValue::make_string(command.file_path);
                    }
                    if (property == "first_arg") {
                        if (command.argument_count() > 0) {
                            return make_argument_value(command.argument(0));
                        }
                        return DslValue::null();
                    }
                    if (property == "last_arg") {
                        if (command.argument_count() > 0) {
                            return make_argument_value(command.argument(command.argument_count() - 1));
                        }
                        return DslValue::null();
                    }
                }
                if (ref.kind == DslNodeRef::Kind::Argument && ref.argument != nullptr) {
                    const xe::cmake::core::ArgumentNode &argument = *ref.argument;
                    if (property == "text") {
                        return DslValue::make_string(argument.text);
                    }
                    if (property == "quote_kind") {
                        std::string kind = "raw";
                        if (argument.quote_kind == xe::cmake::core::QuoteKind::Quoted) {
                            kind = "quoted";
                        } else if (argument.quote_kind == xe::cmake::core::QuoteKind::Bracket) {
                            kind = "bracket";
                        }
                        return DslValue::make_string(kind);
                    }
                    if (property == "is_quoted") {
                        return DslValue::make_bool(argument.is_quoted());
                    }
                    if (property == "is_raw") {
                        return DslValue::make_bool(argument.is_raw());
                    }
                    if (property == "is_bracket") {
                        return DslValue::make_bool(argument.is_bracket());
                    }
                    if (property == "span") {
                        return make_span_value(argument.span);
                    }
                    if (property == "index") {
                        return DslValue::make_int(static_cast<long long>(argument.index));
                    }
                }
                if (ref.kind == DslNodeRef::Kind::Span) {
                    const xe::cmake::core::SourceSpan &span = ref.span;
                    if (property == "start_offset") {
                        return DslValue::make_int(static_cast<long long>(span.start_offset));
                    }
                    if (property == "end_offset") {
                        return DslValue::make_int(static_cast<long long>(span.end_offset));
                    }
                    if (property == "start_line") {
                        return DslValue::make_int(static_cast<long long>(span.start_line));
                    }
                    if (property == "start_column") {
                        return DslValue::make_int(static_cast<long long>(span.start_column));
                    }
                    if (property == "end_line") {
                        return DslValue::make_int(static_cast<long long>(span.end_line));
                    }
                    if (property == "end_column") {
                        return DslValue::make_int(static_cast<long long>(span.end_column));
                    }
                }
                return std::nullopt;
            }

            std::optional<DslValue> evaluate_method(const DslValue &object, const std::string &method, const std::vector<ExprPtr> &args) {
                if (object.kind != DslValue::Kind::Node) {
                    return std::nullopt;
                }
                const DslNodeRef &ref = object.node_value;

                if (ref.kind == DslNodeRef::Kind::Command && ref.command != nullptr) {
                    if (method == "argument") {
                        if (args.size() != 1) {
                            return std::nullopt;
                        }
                        std::optional<DslValue> index = evaluate(args[0]);
                        if (!index.has_value() || index->kind != DslValue::Kind::Int) {
                            return std::nullopt;
                        }
                        const long long idx = index->int_value;
                        if (idx < 0 || static_cast<std::size_t>(idx) >= ref.command->argument_count()) {
                            return DslValue::null();
                        }
                        return make_argument_value(ref.command->argument(static_cast<std::size_t>(idx)));
                    }
                }

                if (ref.kind == DslNodeRef::Kind::File && ref.file != nullptr) {
                    if (method == "has_command") {
                        if (args.size() != 1) {
                            return std::nullopt;
                        }
                        std::optional<DslValue> name = evaluate(args[0]);
                        if (!name.has_value()) {
                            return std::nullopt;
                        }
                        for (const xe::cmake::core::CommandNode *command : ref.file->commands()) {
                            if (command->name == name->string_value) {
                                return DslValue::make_bool(true);
                            }
                        }
                        return DslValue::make_bool(false);
                    }
                    if (method == "count_commands") {
                        if (args.size() != 1) {
                            return std::nullopt;
                        }
                        std::optional<DslValue> name = evaluate(args[0]);
                        if (!name.has_value()) {
                            return std::nullopt;
                        }
                        long long count = 0;
                        for (const xe::cmake::core::CommandNode *command : ref.file->commands()) {
                            if (command->name == name->string_value) {
                                ++count;
                            }
                        }
                        return DslValue::make_int(count);
                    }
                }

                return std::nullopt;
            }

            std::optional<DslValue> evaluate_call(const std::string &callee, const std::vector<ExprPtr> &args) {
                if (callee == "len") {
                    if (args.size() != 1) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> value = evaluate(args[0]);
                    if (!value.has_value() || value->kind != DslValue::Kind::List) {
                        return std::nullopt;
                    }
                    return DslValue::make_int(static_cast<long long>(value->list_value.size()));
                }

                if (callee == "count") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> sequence = evaluate(args[0]);
                    if (!sequence.has_value() || sequence->kind != DslValue::Kind::List) {
                        return std::nullopt;
                    }
                    long long count = 0;
                    for (const DslValue &item : sequence->list_value) {
                        std::optional<DslValue> matched = eval_lambda(args[1], item);
                        if (matched.has_value() && matched->is_truthy()) {
                            ++count;
                        }
                    }
                    return DslValue::make_int(count);
                }

                if (callee == "exists") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> sequence = evaluate(args[0]);
                    if (!sequence.has_value() || sequence->kind != DslValue::Kind::List) {
                        return std::nullopt;
                    }
                    for (const DslValue &item : sequence->list_value) {
                        std::optional<DslValue> matched = eval_lambda(args[1], item);
                        if (matched.has_value() && matched->is_truthy()) {
                            return DslValue::make_bool(true);
                        }
                    }
                    return DslValue::make_bool(false);
                }

                if (callee == "all") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> sequence = evaluate(args[0]);
                    if (!sequence.has_value() || sequence->kind != DslValue::Kind::List) {
                        return std::nullopt;
                    }
                    for (const DslValue &item : sequence->list_value) {
                        std::optional<DslValue> matched = eval_lambda(args[1], item);
                        if (!matched.has_value() || !matched->is_truthy()) {
                            return DslValue::make_bool(false);
                        }
                    }
                    return DslValue::make_bool(true);
                }

                if (callee == "filter") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> sequence = evaluate(args[0]);
                    if (!sequence.has_value() || sequence->kind != DslValue::Kind::List) {
                        return std::nullopt;
                    }
                    std::vector<DslValue> matches;
                    for (const DslValue &item : sequence->list_value) {
                        std::optional<DslValue> matched = eval_lambda(args[1], item);
                        if (matched.has_value() && matched->is_truthy()) {
                            matches.push_back(item);
                        }
                    }
                    return DslValue::make_list(std::move(matches));
                }

                if (callee == "first") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> sequence = evaluate(args[0]);
                    if (!sequence.has_value() || sequence->kind != DslValue::Kind::List) {
                        return std::nullopt;
                    }
                    for (const DslValue &item : sequence->list_value) {
                        std::optional<DslValue> matched = eval_lambda(args[1], item);
                        if (matched.has_value() && matched->is_truthy()) {
                            return item;
                        }
                    }
                    return DslValue::null();
                }

                if (callee == "regex_match" || callee == "regex_search" || callee == "starts_with" || callee == "ends_with" || callee == "contains") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> text = evaluate(args[0]);
                    std::optional<DslValue> pattern = evaluate(args[1]);
                    if (!text.has_value() || !pattern.has_value() || text->kind != DslValue::Kind::String || pattern->kind != DslValue::Kind::String) {
                        return std::nullopt;
                    }
                    if (callee == "regex_match") {
                        return DslValue::make_bool(StringPrimitives::regex_match(text->string_value, pattern->string_value));
                    }
                    if (callee == "regex_search") {
                        return DslValue::make_bool(StringPrimitives::regex_search(text->string_value, pattern->string_value));
                    }
                    if (callee == "starts_with") {
                        return DslValue::make_bool(StringPrimitives::str_starts_with(text->string_value, pattern->string_value));
                    }
                    if (callee == "ends_with") {
                        return DslValue::make_bool(StringPrimitives::str_ends_with(text->string_value, pattern->string_value));
                    }
                    return DslValue::make_bool(StringPrimitives::str_contains(text->string_value, pattern->string_value));
                }

                if (callee == "split") {
                    if (args.size() != 2) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> text = evaluate(args[0]);
                    std::optional<DslValue> delimiter = evaluate(args[1]);
                    if (!text.has_value() || !delimiter.has_value() || text->kind != DslValue::Kind::String || delimiter->kind != DslValue::Kind::String) {
                        return std::nullopt;
                    }
                    std::vector<DslValue> parts;
                    for (std::string &part : StringPrimitives::str_split(text->string_value, delimiter->string_value)) {
                        parts.push_back(DslValue::make_string(std::move(part)));
                    }
                    return DslValue::make_list(std::move(parts));
                }

                if (callee == "to_lower" || callee == "to_upper") {
                    if (args.size() != 1) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> text = evaluate(args[0]);
                    if (!text.has_value() || text->kind != DslValue::Kind::String) {
                        return std::nullopt;
                    }
                    if (callee == "to_lower") {
                        return DslValue::make_string(StringPrimitives::to_lower(text->string_value));
                    }
                    return DslValue::make_string(StringPrimitives::to_upper(text->string_value));
                }

                if (callee == "path_basename" || callee == "path_dirname" || callee == "path_stem" || callee == "path_extension") {
                    if (args.size() != 1) {
                        return std::nullopt;
                    }
                    std::optional<DslValue> path = evaluate(args[0]);
                    if (!path.has_value() || path->kind != DslValue::Kind::String) {
                        return std::nullopt;
                    }
                    const std::filesystem::path fs_path(path->string_value);
                    if (callee == "path_basename") {
                        return DslValue::make_string(fs_path.filename().string());
                    }
                    if (callee == "path_dirname") {
                        return DslValue::make_string(fs_path.parent_path().string());
                    }
                    if (callee == "path_stem") {
                        return DslValue::make_string(fs_path.stem().string());
                    }
                    return DslValue::make_string(fs_path.extension().string());
                }

                return std::nullopt;
            }

            std::optional<DslValue> eval_lambda(const ExprPtr &lambda_expr, const DslValue &item) {
                const LambdaExpr *lambda = dynamic_cast<LambdaExpr *>(lambda_expr.get());
                if (lambda == nullptr) {
                    return evaluate(lambda_expr);
                }
                // Lambda arguments are applied positionally: the caller binds a single
                // parameter per invocation for the sequence functions.
                auto previous = scope_.find(lambda->parameter);
                const bool had_previous = previous != scope_.end();
                DslValue old_value;
                if (had_previous) {
                    old_value = previous->second;
                }
                scope_[lambda->parameter] = item;
                std::optional<DslValue> result = evaluate(lambda->body);
                if (had_previous) {
                    scope_[lambda->parameter] = old_value;
                } else {
                    scope_.erase(lambda->parameter);
                }
                return result;
            }

            std::optional<DslValue> evaluate_binary(const BinaryExpr *binary) {
                switch (binary->op) {
                case BinaryOp::And: {
                    std::optional<DslValue> left = evaluate(binary->left);
                    if (!left.has_value() || !left->is_truthy()) {
                        return DslValue::make_bool(false);
                    }
                    std::optional<DslValue> right = evaluate(binary->right);
                    return DslValue::make_bool(right.has_value() && right->is_truthy());
                }
                case BinaryOp::Or: {
                    std::optional<DslValue> left = evaluate(binary->left);
                    if (left.has_value() && left->is_truthy()) {
                        return DslValue::make_bool(true);
                    }
                    std::optional<DslValue> right = evaluate(binary->right);
                    return DslValue::make_bool(right.has_value() && right->is_truthy());
                }
                case BinaryOp::In:
                case BinaryOp::NotIn: {
                    std::optional<DslValue> left = evaluate(binary->left);
                    std::optional<DslValue> right = evaluate(binary->right);
                    bool member = false;
                    if (left.has_value() && right.has_value() && right->kind == DslValue::Kind::List) {
                        for (const DslValue &element : right->list_value) {
                            if (values_equal(*left, element)) {
                                member = true;
                                break;
                            }
                        }
                    } else if (left.has_value() && right.has_value() && right->kind == DslValue::Kind::String && left->kind == DslValue::Kind::String) {
                        member = right->string_value.find(left->string_value) != std::string::npos;
                    }
                    return DslValue::make_bool(binary->op == BinaryOp::In ? member : !member);
                }
                case BinaryOp::Eq:
                case BinaryOp::Ne: {
                    std::optional<DslValue> left = evaluate(binary->left);
                    std::optional<DslValue> right = evaluate(binary->right);
                    if (!left.has_value() || !right.has_value()) {
                        return DslValue::make_bool(binary->op == BinaryOp::Ne);
                    }
                    const bool equal = values_equal(*left, *right);
                    return DslValue::make_bool(binary->op == BinaryOp::Eq ? equal : !equal);
                }
                case BinaryOp::Lt:
                case BinaryOp::Le:
                case BinaryOp::Gt:
                case BinaryOp::Ge: {
                    std::optional<DslValue> left = evaluate(binary->left);
                    std::optional<DslValue> right = evaluate(binary->right);
                    if (!left.has_value() || !right.has_value()) {
                        return DslValue::make_bool(false);
                    }
                    const long long comparison = compare_values(*left, *right);
                    switch (binary->op) {
                    case BinaryOp::Lt:
                        return DslValue::make_bool(comparison < 0);
                    case BinaryOp::Le:
                        return DslValue::make_bool(comparison <= 0);
                    case BinaryOp::Gt:
                        return DslValue::make_bool(comparison > 0);
                    case BinaryOp::Ge:
                        return DslValue::make_bool(comparison >= 0);
                    default:
                        return DslValue::make_bool(false);
                    }
                }
                }
                return std::nullopt;
            }

            static bool values_equal(const DslValue &left, const DslValue &right) {
                if (left.kind == DslValue::Kind::Int && right.kind == DslValue::Kind::Int) {
                    return left.int_value == right.int_value;
                }
                if (left.kind == DslValue::Kind::String && right.kind == DslValue::Kind::String) {
                    return left.string_value == right.string_value;
                }
                if (left.kind == DslValue::Kind::Bool && right.kind == DslValue::Kind::Bool) {
                    return left.bool_value == right.bool_value;
                }
                if (left.kind == DslValue::Kind::Null && right.kind == DslValue::Kind::Null) {
                    return true;
                }
                if (left.kind == DslValue::Kind::Node && right.kind == DslValue::Kind::Node) {
                    const DslNodeRef &a = left.node_value;
                    const DslNodeRef &b = right.node_value;
                    if (a.kind == DslNodeRef::Kind::Span && b.kind == DslNodeRef::Kind::Span) {
                        return a.span == b.span;
                    }
                }
                return false;
            }

            static long long compare_values(const DslValue &left, const DslValue &right) {
                if (left.kind == DslValue::Kind::Int && right.kind == DslValue::Kind::Int) {
                    return left.int_value - right.int_value;
                }
                if (left.kind == DslValue::Kind::String && right.kind == DslValue::Kind::String) {
                    if (left.string_value < right.string_value) {
                        return -1;
                    }
                    if (left.string_value > right.string_value) {
                        return 1;
                    }
                    return 0;
                }
                return 0;
            }

            std::map<std::string, DslValue> scope_;
        };

    } // namespace

    bool DslPredicateEvaluator::evaluate(std::string_view expression, const DslContext &context) const {
        if (expression.empty() || context.file == nullptr) {
            return false;
        }
        if (context.command == nullptr && (context.argument != nullptr)) {
            // Argument-level rules still need a command context; the caller
            // guarantees context.command is populated for argument matches.
            return false;
        }
        const std::vector<Token> tokens = Lexer(expression).tokenize();
        Parser parser(tokens);
        ExprPtr ast = parser.parse();
        if (ast == nullptr) {
            return false;
        }
        Evaluator evaluator(context);
        std::optional<DslValue> result = evaluator.evaluate(ast);
        return result.has_value() && result->is_truthy();
    }

    std::string DslPredicateEvaluator::interpolate(std::string_view message, const DslContext &context) const {
        std::string result;
        std::size_t pos = 0;
        while (pos < message.size()) {
            const std::size_t open = message.find("${", pos);
            if (open == std::string_view::npos) {
                result.append(message.substr(pos));
                break;
            }
            result.append(message.substr(pos, open - pos));
            const std::size_t close = message.find('}', open + 2);
            if (close == std::string_view::npos) {
                result.append(message.substr(open));
                break;
            }
            const std::string_view expr = message.substr(open + 2, close - open - 2);
            const std::vector<Token> tokens = Lexer(expr).tokenize();
            Parser parser(tokens);
            ExprPtr ast = parser.parse();
            bool appended = false;
            if (ast != nullptr) {
                Evaluator evaluator(context);
                std::optional<DslValue> value = evaluator.evaluate(ast);
                if (value.has_value()) {
                    if (value->kind == DslValue::Kind::String) {
                        result.append(value->string_value);
                        appended = true;
                    } else if (value->kind == DslValue::Kind::Int) {
                        result.append(std::to_string(value->int_value));
                        appended = true;
                    } else if (value->kind == DslValue::Kind::Bool) {
                        result.append(value->bool_value ? "true" : "false");
                        appended = true;
                    }
                }
            }
            if (!appended) {
                // Preserve the original text when the expression cannot be
                // evaluated (e.g. an escaped literal such as \${target}).
                result.append("${");
                result.append(expr);
                result.push_back('}');
            }
            pos = close + 1;
        }
        return result;
    }

} // namespace xe::cmake::dsl