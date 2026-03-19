#include "commands/CopyCommand.hpp"
#include "display/Renderer.hpp"

namespace snip {

void CopyCommand::registerTo(CLI::App& app, SnippetService& service) {
    auto* sub = app.add_subcommand("copy", "Copy a snippet's content to clipboard");

    auto* id_opt = new int(0);

    sub->add_option("id", *id_opt, "Snippet ID")
       ->required();

    sub->callback([&service, id_opt]() {
        try {
            const std::string title = service.copyToClipboard(*id_opt);
            Renderer::printSuccess("Copied to clipboard — \"" + title + "\"");
        } catch (const std::exception& e) {
            Renderer::printError(std::string("Copy failed: ") + e.what());
        }

        delete id_opt;
    });
}

} // namespace snip
