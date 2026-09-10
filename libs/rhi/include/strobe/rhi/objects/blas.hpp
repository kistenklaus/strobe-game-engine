#pragma once

#include "strobe/rhi/objects/buffer.hpp"

namespace strobe::rhi {


/**
 * \ingroup rhi
 * \brief Bottom Level Acceleration Structure
 * 
 * Blas is a BVH acceleration structure, containing triangles or AABBs.
 */
struct Blas : public Object<Blas> {
  friend class Device;
  friend class MemoryPool;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  /** \name (constructors)
   * @{
   */
  /**
   * \brief default constructor
   */
  Blas() noexcept : Object(nullptr) {}
  /**
   * \brief copy-constructor
   */
  Blas(const Blas &) noexcept;
  /**
   * \brief move-constructor
   */
  Blas(Blas &&) noexcept;
  /**
   * \brief copy-assignment
   */
  Blas &operator=(const Blas &) noexcept;
  /**
   * \brief move-assignment
   */
  Blas &operator=(Blas &&) noexcept;
  /**
   * \brief destructor
   */
  ~Blas() noexcept;

  explicit Blas(void *handle) noexcept : Object(handle) {}
private:
};

} // namespace strobe::rhi
