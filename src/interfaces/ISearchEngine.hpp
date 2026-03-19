#pragma once

#include <string>
#include <vector>
#include "models/Snippet.hpp"

namespace snip {

/// 搜索引擎纯虚接口
/// 支持在给定语料库中执行关键字或语义检索
class ISearchEngine {
public:
    virtual ~ISearchEngine() = default;

    /// 在 corpus 中检索匹配 query 的代码片段，返回结果集
    /// @param query   搜索关键字或表达式
    /// @param corpus  待检索的代码片段集合
    virtual std::vector<Snippet> search(
        const std::string&         query,
        const std::vector<Snippet>& corpus
    ) = 0;

    /// 是否支持流式输出（如 AI 逐字返回场景），默认不支持
    virtual bool supportsStreaming() const { return false; }
};

} // namespace snip
