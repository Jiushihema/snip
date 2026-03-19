#pragma once

#include <string>
#include <vector>
#include <optional>
#include <SQLiteCpp/SQLiteCpp.h>
#include "interfaces/IStore.hpp"
#include "models/Snippet.hpp"
#include <optional>

namespace snip {

/// IStore 的 SQLite 持久化实现
/// 使用 SQLiteCpp 封装，构造时自动建库建表
class SqliteStore : public IStore {
public:
    /// @param db_path  数据库文件的完整路径（含文件名），目录不存在时自动创建
    explicit SqliteStore(const std::string& db_path);

    ~SqliteStore() override = default;

    // 禁止拷贝，允许移动
    SqliteStore(const SqliteStore&)            = delete;
    SqliteStore& operator=(const SqliteStore&) = delete;
    SqliteStore(SqliteStore&&)                 = default;
    SqliteStore& operator=(SqliteStore&&)      = default;

    Snippet                  add(const Snippet& snippet)  override;
    std::vector<Snippet>     getAll()                     override;
    std::optional<Snippet>   getById(int id)              override;
    bool                     remove(int id)               override;
    bool                     update(const Snippet& snippet) override;

private:
    SQLite::Database db_;

    /// 执行建表 DDL（幂等）
    void initSchema();

    /// 将一行查询结果映射为 Snippet
    static Snippet rowToSnippet(SQLite::Statement& stmt);
};

} // namespace snip
