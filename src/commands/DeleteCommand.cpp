#include "commands/DeleteCommand.hpp"
#include "display/Renderer.hpp"

#include <iostream>
#include <string>

namespace snip {

void DeleteCommand::registerTo(CLI::App& app, SnippetService& service) {
    auto* sub = app.add_subcommand("delete", "Delete a snippet by ID");

    auto* id_opt = new int(0);

    sub->add_option("id", *id_opt, "Snippet ID to delete")
       ->required();

    sub->callback([&service, id_opt]() {
        const int id = *id_opt;
        delete id_opt;

        // 删除前展示片段摘要，让用户确认
        try {
            const auto snippet = service.getById(id);
            Renderer::printWarning(
                "About to delete snippet ID " + std::to_string(id) +
                " — \"" + snippet->title + "\"");
        } catch (const std::exception&) {
            Renderer::printError(
                "Snippet with ID " + std::to_string(id) + " does not exist.");
            return;
        }

        // y/n 确认
        Renderer::printWarning("Are you sure? [y/N] ");
        std::string answer;
        std::getline(std::cin, answer);

        if (answer != "y" && answer != "Y") {
            Renderer::printWarning("Aborted.");
            return;
        }

        try {
            const bool ok = service.deleteSnippet(id);
            if (ok) {
                Renderer::printSuccess(
                    "Snippet ID " + std::to_string(id) + " deleted.");
            } else {
                Renderer::printError(
                    "Delete failed — snippet ID " +
                    std::to_string(id) + " not found.");
            }
        } catch (const std::exception& e) {
            Renderer::printError(std::string("Delete failed: ") + e.what());
        }
    });
}

} // namespace snip
