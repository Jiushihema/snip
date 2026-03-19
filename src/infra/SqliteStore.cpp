#include "infra/SqliteStore.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>
// 在文件顶部加上这个辅助函数（namespace snip 外面）
static std::string getDefaultDbPath() {
    const char* home = std::getenv("USERPROFILE"); // Windows
    if (!home) home = std::getenv("HOME");          // Linux/macOS
    if (!home) home = ".";
    
    std::filesystem::path dir = std::filesystem::path(home) / ".config" / "snip";
    std::filesystem::create_directories(dir);
    return (dir / "snip.db").string();
}

namespace snip {

// ─────────────────────────────────────────────
// 构造 & 初始化
// ─────────────────────────────────────────────

SqliteStore::SqliteStore(const std::string& db_path)
    : db_(db_path.empty() ? getDefaultDbPath() : db_path,
          SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    // 目录已在 getDefaultDbPath() 里创建，这里只做额外保险
    const std::filesystem::path path(db_.getFilename());
    if (const auto parent = path.parent_path(); !parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    db_.exec("PRAGMA journal_mode=WAL;");
    db_.exec("PRAGMA foreign_keys=ON;");
    initSchema();
}


void SqliteStore::initSchema() {
    db_.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS snippets (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            title       TEXT    NOT NULL DEFAULT '',
            content     TEXT    NOT NULL DEFAULT '',
            language    TEXT    NOT NULL DEFAULT '',
            tags        TEXT    NOT NULL DEFAULT '',
            created_at  TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S', 'now'))
        );
    )SQL");
}

// ─────────────────────────────────────────────
// 私有辅助
// ─────────────────────────────────────────────

Snippet SqliteStore::rowToSnippet(SQLite::Statement& stmt) {
    Snippet s;
    s.id         = stmt.getColumn("id").getInt();
    s.title      = stmt.getColumn("title").getString();
    s.content    = stmt.getColumn("content").getString();
    s.language   = stmt.getColumn("language").getString();
    s.tags       = stmt.getColumn("tags").getString();
    s.created_at = stmt.getColumn("created_at").getString();
    return s;
}

// ─────────────────────────────────────────────
// IStore 接口实现
// ─────────────────────────────────────────────

Snippet SqliteStore::add(const Snippet& snippet) {
    SQLite::Statement stmt(db_,
        "INSERT INTO snippets (title, content, language, tags, created_at) "
        "VALUES (?, ?, ?, ?, strftime('%Y-%m-%dT%H:%M:%S', 'now'));");

    stmt.bind(1, snippet.title);
    stmt.bind(2, snippet.content);
    stmt.bind(3, snippet.language);
    stmt.bind(4, snippet.tags);

    stmt.exec();

    // 返回携带自增 id 的完整记录
    const int new_id = static_cast<int>(db_.getLastInsertRowid());

    // getById 在此处一定能找到，直接解引用
    return *getById(new_id);
}

std::vector<Snippet> SqliteStore::getAll() {
    SQLite::Statement stmt(db_,
        "SELECT id, title, content, language, tags, created_at "
        "FROM snippets ORDER BY id ASC;");

    std::vector<Snippet> results;
    while (stmt.executeStep()) {
        results.push_back(rowToSnippet(stmt));
    }
    return results;
}

std::optional<Snippet> SqliteStore::getById(int id) {
    SQLite::Statement stmt(db_,
        "SELECT id, title, content, language, tags, created_at "
        "FROM snippets WHERE id = ?;");

    stmt.bind(1, id);

    if (stmt.executeStep()) {
        return rowToSnippet(stmt);
    }

    // 按接口约定：找不到时抛出异常
    throw std::runtime_error(
        "Snippet with id " + std::to_string(id) + " not found.");
}

bool SqliteStore::remove(int id) {
    SQLite::Statement stmt(db_,
        "DELETE FROM snippets WHERE id = ?;");

    stmt.bind(1, id);
    stmt.exec();

    // getChanges() 返回受影响行数，>0 表示删除成功
    return db_.getChanges() > 0;
}

bool SqliteStore::update(const Snippet& snippet) {
    SQLite::Statement stmt(db_,
        "UPDATE snippets "
        "SET title = ?, content = ?, language = ?, tags = ? "
        "WHERE id = ?;");

    stmt.bind(1, snippet.title);
    stmt.bind(2, snippet.content);
    stmt.bind(3, snippet.language);
    stmt.bind(4, snippet.tags);
    stmt.bind(5, snippet.id);

    stmt.exec();

    return db_.getChanges() > 0;
}

} // namespace snip
