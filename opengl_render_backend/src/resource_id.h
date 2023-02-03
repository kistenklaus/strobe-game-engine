#pragma once

#include <cassert>
#include <numeric>

namespace strobe::internal {

    typedef unsigned int resource_id_type;

    class resource_id {
    private:
        const static resource_id_type NULL_ID = std::numeric_limits<resource_id_type>::max();
    public:

        resource_id() : m_id(NULL_ID) {}

        explicit resource_id(resource_id_type id)
                : m_id(id) {
            assert(m_id != NULL_ID);
        }

        const resource_id_type &operator*() const {
            return m_id;
        }

        bool operator==(const resource_id &other) const {
            return m_id == other.m_id;
        }

        bool operator!=(const resource_id &other) const {
            return m_id != other.m_id;
        }

        bool operator>(const resource_id &other) const {
            return m_id > other.m_id;
        }

        bool operator<(const resource_id &other) const {
            return m_id < other.m_id;
        }

        bool operator>=(const resource_id &other) const {
            return m_id >= other.m_id;
        }

        bool operator<=(const resource_id &other) const {
            return m_id <= other.m_id;
        }

        [[nodiscard]] explicit operator bool() const {
            return m_id != NULL_ID;
        }

        [[nodiscard]] bool isValid() const {
            return m_id != NULL_ID;
        }

        [[nodiscard]] explicit operator resource_id_type() const{
            return m_id;
        }

        [[nodiscard]] unsigned int asIntegral() const {
            return static_cast<unsigned int>(m_id);
        }

    private:
        resource_id_type m_id;
    };

    const resource_id null_id = resource_id();

}