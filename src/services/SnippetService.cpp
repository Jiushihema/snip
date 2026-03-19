#include "services/SnippetService.hpp"

// 本文件严格禁止 include 任何 infra/ 层头文件
// 所有能力均通过 IStore / ISearchEngine / IClipboard 接口访问

namespace snip {

// ─────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────

SnippetService::SnippetService(
    std::shared_ptr<IStore>        store,
    std::shared_ptr<ISearchEngine> searchEngine,
    std::shared_ptr<IClipboard>    clipboard)
    : store_(std::move(store))
    , searchEngine_(std::move(searchEngine))
    , clipboard_(std::move(clipboard))
{}

// ─────────────────────────────────────────────
// 业务方法实现
// ─────────────────────────────────────────────

Snippet SnippetService::addSnippet(
    const std::string& title,
    const std::string& content,
    const std::string& language,
    const std::string& tags)
{
    Snippet s;
    s.title    = title;
    s.content  = content;
    s.language = language;
    s.tags     = tags;
    // created_at 由 store 层（数据库默认值）填充，此处无需设置
    return store_->add(s);
}

std::vector<Snippet> SnippetService::listAll() {
    return store_->getAll();
}

std::vector<Snippet> SnippetService::search(const std::string& query) {
    // 先取全量数据，再交由搜索引擎过滤
    // 搜索引擎不感知存储层，保持职责分离
    const std::vector<Snippet> corpus = store_->getAll();
    return searchEngine_->search(query, corpus);
}

std::string SnippetService::copyToClipboard(int id) {
    // getById 在 id 不存在时由 IStore 实现抛出 std::runtime_error
    // 此处不捕获，让异常向上传播至 CLI 层统一处理
    const std::optional<Snippet> snippet = store_->getById(id);

    // 将片段内容写入剪贴板
    clipboard_->copy(snippet->content);

    // 返回标题，供 CLI 层展示 "已复制：xxx" 的确认信息
    return snippet->title;
}

bool SnippetService::deleteSnippet(int id) {
    return store_->remove(id);
}

std::optional<Snippet> SnippetService::getById(int id) {
    // 透传至 store，异常由 store 层负责抛出
    return store_->getById(id);
}

} // namespace snip
