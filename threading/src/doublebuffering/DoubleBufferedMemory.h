#pragma once

namespace strobe {

    template<typename T>
    class DoubleBufferedMemory {
    public:
        explicit DoubleBufferedMemory(unsigned int size)
                : m_size(size),
                  m_data(new T[2 * size]),
                  m_front(m_data),
                  m_back(m_data + size) {}

        ~DoubleBufferedMemory() {
            delete[] m_data;
        }

        T *front() {
            return m_front;
        }

        T *back() {
            return m_back;
        }

        void swapBuffers() {
            T *tmp = m_front;
            m_front = m_back;
            m_back = tmp;
        }

        [[nodiscard]] unsigned int size() const {
            return m_size;
        }

    private:
        unsigned int m_size;
        T *m_data;
        T *m_front;
        T *m_back;
    };

}