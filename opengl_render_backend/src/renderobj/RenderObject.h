#pragma once

#include "../resource_id.h"

namespace strobe {

    class RenderObject {
        friend class RenderObjectQueue;
    public:
        explicit RenderObject()= default;
        internal::resource_id m_geometrieId;
    };

}