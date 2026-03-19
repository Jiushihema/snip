#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "interfaces/IStore.hpp"
#include "interfaces/ISearchEngine.hpp"
#include "interfaces/IClipboard.hpp"
#include "models/Snippet.hpp"
#include <optional>

namespace snip {

/// 业务逻辑层：代码片段管理服务
/// 所有依赖均通过构造函数注入，不持有任何 infra 层具体类型
class SnippetService {
public:
    /// @param store         持久化存储接口
    /// @param searchEngine  搜索引擎接口
    /// @param clipboard     剪贴板接口
    SnippetService(
        std::shared_ptr<IStore>        store,
        std::shared_ptr<ISearchEngine> searchEngine,
        std::shared_ptr<IClipboard>    clipboard
    );

    ~SnippetService() = default;

    // 禁止拷贝，允许移动
    SnippetService(const SnippetService&)            = delete;
    SnippetService& operator=(const SnippetService&) = delete;
    SnippetService(SnippetService&&)                 = default;
    SnippetService& operator=(SnippetService&&)      = default;

    /// 新增代码片段，返回写入后携带自增 id 的完整 Snippet
    /// @param tags  逗号分隔的标签字符串，如 "cpp,algo"
    Snippet addSnippet(
        const std::string& title,
        const std::string& content,
        const std::string& language,
        const std::string& tags
    );

    /// 返回所有代码片段（按 id 升序）
    std::vector<Snippet> listAll();

    /// 在全量数据中执行搜索，返回匹配结果
    /// @param query  搜索关键字
    std::vector<Snippet> search(const std::string& query);

    /// 将指定片段的 content 复制到系统剪贴板
    /// @param id  片段 id
    /// @return    被复制片段的 title，供调用方展示确认信息
    /// @throws    std::runtime_error  当 id 不存在时（由 IStore 抛出）
    std::string copyToClipboard(int id);

    /// 删除指定片段，成功返回 true，id 不存在返回 false
    bool deleteSnippet(int id);

    /// 根据 id 获取单条片段
    /// @throws std::runtime_error  当 id 不存在时（由 IStore 抛出）
    std::optional<Snippet> getById(int id);

private:
    std::shared_ptr<IStore>        store_;
    std::shared_ptr<ISearchEngine> searchEngine_;
    std::shared_ptr<IClipboard>    clipboard_;
};

} // namespace snip
