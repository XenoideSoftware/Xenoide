#include <charconv>
#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>
#include <lsp/connection.h>
#include <lsp/io/socket.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>

/*
 * Minimal LSP server POC using lsp-framework.
 *
 * Usage:
 *   poc-lsp-server              -- stdio mode (for editor integration)
 *   poc-lsp-server --port=12345 -- socket mode (for poc-lsp-client)
 *
 * Handles: initialize, textDocument/hover, shutdown, exit.
 */

namespace {

thread_local bool g_running = false;

void registerCallbacks(lsp::MessageHandler& handler)
{
    handler.add<lsp::requests::Initialize>(
        [](lsp::requests::Initialize::Params&& params)
        {
            std::cerr << "[server] initialize\n";
            return lsp::requests::Initialize::Result{
                .capabilities = {
                    .textDocumentSync = lsp::TextDocumentSyncOptions{
                        .openClose = true,
                        .change    = lsp::TextDocumentSyncKind::Full,
                        .save      = true
                    },
                    .hoverProvider = true,
                },
                .serverInfo = lsp::InitializeResultServerInfo{
                    .name    = "poc-lsp-server",
                    .version = "0.1.0"
                },
            };
        }
    ).add<lsp::requests::TextDocument_Hover>(
        [](lsp::requests::TextDocument_Hover::Params&& params)
        {
            std::cerr << "[server] textDocument/hover\n";
            return std::async(std::launch::deferred,
                [params = std::move(params)]()
                {
                    return lsp::requests::TextDocument_Hover::Result(
                        lsp::Hover{ .contents = "Hello from poc-lsp-server!" }
                    );
                }
            );
        }
    ).add<lsp::requests::Shutdown>(
        []()
        {
            std::cerr << "[server] shutdown\n";
            return lsp::requests::Shutdown::Result();
        }
    ).add<lsp::notifications::Exit>(
        []()
        {
            std::cerr << "[server] exit\n";
            g_running = false;
        }
    );
}

void runServer(lsp::io::Stream& io)
{
    try
    {
        auto connection     = lsp::Connection(io);
        auto messageHandler = lsp::MessageHandler(connection);
        registerCallbacks(messageHandler);

        g_running = true;
        while(g_running)
            messageHandler.processIncomingMessages();
    }
    catch(const std::exception& e)
    {
        std::cerr << "[server] ERROR: " << e.what() << '\n';
    }
}

void runSocketServer(unsigned short port)
{
    std::cerr << "[server] Listening on port " << port << '\n';
    auto listener = lsp::io::SocketListener(port);

    while(listener.isReady())
    {
        auto socket = listener.listen();
        if(!socket.isOpen())
            break;

        std::cerr << "[server] Client connected\n";
        std::thread([s = std::move(socket)]() mutable { runServer(s); }).detach();
    }
}

std::optional<unsigned short> parsePort(int argc, char** argv)
{
    constexpr auto prefix = std::string_view("--port=");
    for(int i = 1; i < argc; ++i)
    {
        auto arg = std::string_view(argv[i]);
        if(arg.starts_with(prefix))
        {
            unsigned short port{};
            auto s = arg.substr(prefix.size());
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), port);
            (void)ptr;
            if(ec == std::errc{}) return port;
        }
    }
    return std::nullopt;
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        if(auto port = parsePort(argc, argv))
        {
            runSocketServer(*port);
        }
        else
        {
            std::cerr << "[server] Starting stdio server (use --port=N for socket mode)\n";
            runServer(lsp::io::standardIO());
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "[server] FATAL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
