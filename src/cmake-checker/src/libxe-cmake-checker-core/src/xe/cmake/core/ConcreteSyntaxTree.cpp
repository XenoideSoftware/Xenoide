#include "ConcreteSyntaxTree.h"

#include <vector>

namespace xe::cmake::core {

    std::vector<const CommandNode *> ConcreteSyntaxTree::commands() const {
        std::vector<const CommandNode *> result;
        for (const StatementNode &statement : statements_) {
            if (statement.is_command) {
                result.push_back(&statement.command);
            }
        }
        return result;
    }

} // namespace xe::cmake::core