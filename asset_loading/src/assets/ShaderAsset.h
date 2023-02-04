#pragma once

#include "TextFileAsset.h"
#include <memory>
#include <string>

namespace strobe {
    class ShaderAsset {
    public:

        ShaderAsset() = delete;

        explicit ShaderAsset(std::shared_ptr<TextFileAsset> vertexAsset,
                             std::shared_ptr<TextFileAsset> fragmentAsset)
                : m_vertexAsset(std::move(vertexAsset)),
                  m_fragmentAsset(std::move(fragmentAsset)) {
        }

        [[nodiscard]] const std::string &vertexSource() const {
            return m_vertexAsset->content();
        }

        [[nodiscard]] const std::string &vertexPath() const {
            return m_vertexAsset->path();
        }

        [[nodiscard]] const std::string &fragmentSource() const {
            return m_fragmentAsset->content();
        }

        [[nodiscard]] const std::string &fragmentPath() const {
            return m_fragmentAsset->path();
        }

        [[nodiscard]] bool operator==(const ShaderAsset &other) const {
            return (*m_fragmentAsset) == (*other.m_fragmentAsset)
                   && (*m_vertexAsset) == (*other.m_vertexAsset);
        }

    private:
        std::shared_ptr<TextFileAsset> m_vertexAsset;
        std::shared_ptr<TextFileAsset> m_fragmentAsset;
    };

}