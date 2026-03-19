#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#include "commands/ListCommand.hpp"
#include "display/Renderer.hpp"

namespace snip {

void ListCommand::registerTo(CLI::App& app, SnippetService& service) {
    auto* sub = app.add_subcommand("list", "List all saved snippets");

    sub->callback([&service]() {
        try {
            const auto snippets = service.listAll();
            Renderer::printSnippetList(snippets);
        } catch (const std::exception& e) {
            Renderer::printError(std::string("Failed to list snippets: ") + e.what());
        }
    });
}

} // namespace snip
