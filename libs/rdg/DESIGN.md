# Strobe RDG

RDG is the RHI-specific layer built on `strobe::flow`. Flow owns recording,
value versioning, feedback, cutoff memoization, scheduling, host storage, and
execution. RDG translates compiled Flow decisions into RHI concepts.

`rdg::buffer` and `rdg::image` record ordinary Flow construction operations.
Their callbacks receive the logical `flow::Lifetime` selected by the compiler
and pass its equivalent `rhi::MemoryLifetime` to the RHI. Flow lifetimes are
half-open; RHI memory lifetimes are inclusive. Persistent resources map to
`[0, UINT32_MAX]`. A cutoff on creation information therefore prevents equal
descriptions from reconstructing the RHI handle, while a changed description
creates a new handle and lets the RHI retire the previous allocation safely.

`rdg::begin` installs the RDG compiler plugins in addition to any user plugins.
The current barrier pass records every producer-consumer and sequential-order
hazard involving an `rhi::Buffer` or `rhi::Image`. Its `BarrierPlan` is retained
in `flow::detail::ProgramData::plugin_data`, so later RDG compilation and execution stages can
consume it without global state.

Flow read/write modes determine ordering and value dependencies, while the
two-argument RDG `read`, `write`, and `read_write` overloads associate an
`rhi::AccessScope` with an access. `rdg::op` records those scopes and the
barrier pass materializes a concrete `rhi::MemoryBarrier` when both operations
provide them. Hazards involving unscoped Flow operations remain in the plan
with `materialized == false`; a later lowering pass can reject them or handle
special operations such as resource creation without guessing an expensive
all-commands scope.

Serializable programs will store symbolic external operations, external
resource names, stable resource schemas, and RDG access metadata. Runtime
lambdas and retained typed plugin data are process-local and are not part of
that format.
