#pragma once

#include <vector>
#include <optional>
#include "models/Snippet.hpp"
#include <optional>

namespace snip {

/// 数据持久化层纯虚接口
/// 所有具体存储实现（SQLite、内存、文件等）必须继承此接口
class IStore {
public:
    virtual ~IStore() = default;

    /// 新增一条代码片段，返回写入后携带自增 id 的 Snippet
    virtual Snippet add(const Snippet& snippet) = 0;

    /// 获取所有代码片段
    virtual std::vector<Snippet> getAll() = 0;

    /// 根据 id 获取单条代码片段，不存在时返回 std::nullopt
    virtual std::optional<Snippet> getById(int id) = 0;

    /// 根据 id 删除代码片段，成功返回 true
    virtual bool remove(int id) = 0;

    /// 更新已有代码片段（以 snippet.id 为主键），成功返回 true
    virtual bool update(const Snippet& snippet) = 0;
};

} // namespace snip
