#pragma once

#include <semaphore>

namespace strobe {

    class MemoryGuard {
    public:
        explicit MemoryGuard(std::binary_semaphore *semEnter,
                             std::binary_semaphore *semExit)
                : m_semEnter(semEnter),
                  m_semExit(semExit) {
            m_semEnter->acquire();
        }
        ~MemoryGuard(){
            m_semExit->release();
        }


    private:
        std::binary_semaphore *m_semEnter;
        std::binary_semaphore *m_semExit;
    };
}
