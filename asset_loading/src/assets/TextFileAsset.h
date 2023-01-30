#pragma once

#include <string>


class TextFileAsset {
public:
    explicit TextFileAsset(const std::string &path);

    ~TextFileAsset() = default;

    [[nodiscard]] inline const std::string &content() const {return m_content;}
    [[nodiscard]] inline const std::string &path() const { return m_path; }

private:
    std::string m_content;
    std::string m_path;
};
