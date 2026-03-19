#include "commands/SearchCommand.hpp"
#include "display/Renderer.hpp"

#include <string>

namespace snip {

void SearchCommand::registerTo(CLI::App& app, SnippetService& service) {
    auto* sub = app.add_subcommand("search", "Search snippets by keyword");

    auto* keyword_opt = new std::string();

    sub->add_option("keyword", *keyword_opt, "Search keyword")
       ->required();

    sub->callback([&service, keyword_opt]() {
        try {
            const auto results = service.search(*keyword_opt);

            if (results.empty()) {
                Renderer::printWarning(
                    "No snippets matched \"" + *keyword_opt + "\".");
            } else {
                Renderer::printSnippetList(results);
            }
        } catch (const std::exception& e) {
            Renderer::printError(std::string("Search failed: ") + e.what());
        }

        delete keyword_opt;
    });
}

} // namespace snip
