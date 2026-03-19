#include "commands/AddCommand.hpp"
#include "display/Renderer.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

namespace snip {

void AddCommand::registerTo(CLI::App& app, SnippetService& service) {
    auto* sub = app.add_subcommand("add", "Add a new code snippet (content read from stdin)");

    auto* title_opt = new std::string();
    auto* lang_opt  = new std::string();
    auto* tags_opt  = new std::string();

    sub->add_option("title", *title_opt, "Snippet title")
       ->required();

    sub->add_option("-l,--lang", *lang_opt, "Programming language (e.g. cpp, python)")
       ->default_val("");

    sub->add_option("-t,--tags", *tags_opt, "Comma-separated tags (e.g. algo,sort)")
       ->default_val("");

    sub->callback([&service, title_opt, lang_opt, tags_opt]() {
        // 从 stdin 读取代码内容（支持管道与重定向）
        std::string content{
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>()
        };

        // 去除末尾多余换行
        while (!content.empty() && content.back() == '\n') {
            content.pop_back();
        }

        try {
            const Snippet s = service.addSnippet(
                *title_opt, content, *lang_opt, *tags_opt);

            Renderer::printSuccess(
                "Snippet added — ID: " + std::to_string(s.id) +
                "  Title: " + s.title);
        } catch (const std::exception& e) {
            Renderer::printError(std::string("Failed to add snippet: ") + e.what());
        }

        delete title_opt;
        delete lang_opt;
        delete tags_opt;
    });
}

} // namespace snip
