#pragma once

#include <string>
#include <vector>
#include "models/Snippet.hpp"

namespace snip {

/// 终端渲染器：所有终端输出均通过此类，集中管理颜色与格式
class Renderer {
public:
    Renderer()  = delete;
    ~Renderer() = delete;

    /// 输出代码片段列表（表格形式）
    static void printSnippetList(const std::vector<Snippet>& snippets);

    /// 输出单个片段的完整详情（含 content 代码块）
    static void printSnippet(const Snippet& snippet);

    /// 绿色成功消息
    static void printSuccess(const std::string& msg);

    /// 红色错误消息
    static void printError(const std::string& msg);

    /// 黄色警告消息
    static void printWarning(const std::string& msg);

private:
    /// 将字符串截断到指定显示宽度（超出时末尾加 "…"）
    static std::string truncate(const std::string& s, std::size_t width);

    /// 返回固定宽度的左对齐字符串（不足补空格，超出截断）
    static std::string cell(const std::string& s, std::size_t width);
};

} // namespace snip
