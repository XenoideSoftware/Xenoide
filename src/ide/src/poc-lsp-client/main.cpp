#include <charconv>
#include <iostream>
#include <string_view>
#include <thread>
#include <lsp/connection.h>
#include <lsp/io/socket.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>
#include <lsp/process.h>

/*
 * Minimal LSP client POC using lsp-framework.
 *
 * Usage:
 *   poc-lsp-client --port=12345            -- connect to a socket server
 *   poc-lsp-client --exe=poc-lsp-server    -- spawn server process via stdio
 *
 * Sends: initialize, initialized, textDocument/hover, shutdown, exit.
 */

namespace {

    bool g_running = false;

    std::thread startProcessingThread(lsp::MessageHandler &handler) {
        return std::thread([&handler]() {
            g_running = true;
            while (g_running)
                handler.processIncomingMessages();
        });
    }

    void runClient(lsp::io::Stream &io) {
        auto connection = lsp::Connection(io);
        auto messageHandler = lsp::MessageHandler(connection);
        auto thread = startProcessingThread(messageHandler);

        // initialize
        auto initParams = lsp::requests::Initialize::Params();
        initParams.rootUri = lsp::DocumentUri::fromPath(".");
        initParams.capabilities = {.textDocument = lsp::TextDocumentClientCapabilities{.hover = lsp::HoverClientCapabilities{.contentFormat = {{lsp::MarkupKind::PlainText}}}}};

        auto initFuture = messageHandler.sendRequest<lsp::requests::Initialize>(std::move(initParams));
        auto initResult = initFuture.result.get();
        std::cerr << "[client] Server initialized: " << initResult.serverInfo->name << '\n';

        // initialized notification
        messageHandler.sendNotification<lsp::notifications::Initialized>(lsp::notifications::Initialized::Params{});

        // textDocument/hover
        if (initResult.capabilities.hoverProvider.has_value()) {
            try {
                auto hoverParams = lsp::requests::TextDocument_Hover::Params();
                hoverParams.textDocument.uri = lsp::DocumentUri::fromPath("main.cpp");
                hoverParams.position = {.line = 0, .character = 0};

                auto hoverFuture = messageHandler.sendRequest<lsp::requests::TextDocument_Hover>(std::move(hoverParams));
                auto hoverResult = hoverFuture.result.get();
                std::cerr << "[client] Hover response received\n";
            } catch (const lsp::ResponseError &e) {
                std::cerr << "[client] Hover error: " << e.what() << '\n';
            }
        }

        // shutdown + exit
        messageHandler.sendRequest<lsp::requests::Shutdown>(
            [&messageHandler](lsp::requests::Shutdown::Result &&) {
                std::cerr << "[client] Shutdown acknowledged\n";
                messageHandler.sendNotification<lsp::notifications::Exit>();
                g_running = false;
            },
            [](const lsp::ResponseError &e) {
                std::cerr << "[client] Shutdown error: " << e.what() << '\n';
                g_running = false;
            }
        );

        thread.join();
    }

    struct Args {
        std::optional<unsigned short> port;
        std::string executable;
        std::vector<std::string> executableArgs;
    };

    Args parseArgs(int argc, char **argv) {
        constexpr auto portPrefix = std::string_view("--port=");
        constexpr auto exePrefix = std::string_view("--exe=");

        Args args;
        for (int i = 1; i < argc; ++i) {
            auto arg = std::string_view(argv[i]);
            if (!args.executable.empty()) {
                args.executableArgs.push_back(std::string(arg));
            } else if (arg.starts_with(portPrefix)) {
                unsigned short port{};
                auto s = arg.substr(portPrefix.size());
                auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), port);
                (void)ptr;
                if (ec == std::errc{})
                    args.port = port;
            } else if (arg.starts_with(exePrefix)) {
                args.executable = std::string(arg.substr(exePrefix.size()));
            }
        }
        return args;
    }

} // namespace

int main(int argc, char **argv) {
    auto args = parseArgs(argc, argv);

    if (!args.port.has_value() && args.executable.empty()) {
        std::cerr << "Usage:\n"
                  << "  poc-lsp-client --port=<N>         connect via socket\n"
                  << "  poc-lsp-client --exe=<path> <args> spawn server via stdio\n";
        return 1;
    }

    try {
        if (args.port.has_value()) {
            std::cerr << "[client] Connecting to port " << *args.port << '\n';
            auto socket = lsp::io::Socket::connect(lsp::io::Socket::Localhost, *args.port);
            runClient(socket);
        } else {
            std::cerr << "[client] Spawning '" << args.executable << "'\n";
            auto proc = lsp::Process(args.executable, args.executableArgs);
            runClient(proc.stdIO());
        }
    } catch (const std::exception &e) {
        std::cerr << "[client] FATAL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
