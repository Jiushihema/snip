#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace snip {

struct Snippet {
    int         id         = 0;
    std::string title;
    std::string content;
    std::string language;
    std::string tags;       // 逗号分隔，如 "cpp,algo,sort"
    std::string created_at;

    /// 将逗号分隔的 tags 字符串拆分为 vector<string>
    std::vector<std::string> tagsAsVector() const {
        std::vector<std::string> result;
        if (tags.empty()) return result;

        std::istringstream stream(tags);
        std::string token;
        while (std::getline(stream, token, ',')) {
            // 去除首尾空格
            const auto start = token.find_first_not_of(" \t");
            const auto end   = token.find_last_not_of(" \t");
            if (start != std::string::npos) {
                result.push_back(token.substr(start, end - start + 1));
            }
        }
        return result;
    }
};

} // namespace snip
