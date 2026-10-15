# Flow design contract

Flow records a program rather than a single frame. Resources identify typed
storage locations. Compiler-created value versions identify the values that
flow through those locations. Operations declare their reads and writes, and
the compiler derives data dependencies, resource ordering, storage lifetimes,
aliasing, and scheduling information.

## Recording

Recording is thread-local and delimited by `flow::begin()` and `flow::end()`.
Compilation options and compiler plugins are supplied to `begin()` and copied
into that recording. `end()` validates and compiles using the captured
configuration. Every resource used by a recording belongs to that recording.
An owned resource has exactly one recorded construction operation. Destruction
is inserted by the compiler according to the resource lifetime.

`read` consumes the current value. `write` may preserve parts of the current
value and is therefore ordered after previous accesses. `read_write` exposes
both behaviors explicitly. `discard` invalidates the current value without
running its destructor and permits a later write to define a value without a
dependency on the discarded value.

External resources are borrowed and are identified by name. The program does
not own or synchronize them. Their pointers are bound before submission, and
the owner keeps them alive while submitted work can access them.

Operations recorded with a lambda retain a runtime-only callback. Operations
recorded with `flow::external_op` retain a library ID, symbol, ABI version,
metadata, and argument type information. The application binds a borrowed
`flow::Library` resolver to each referenced library ID before submission. Flow
resolves every symbol for a library before changing any callback and then
normalizes the resolved functions into the same executable callback type used
by lambda operations. It does not interpret a library ID as a path or asset
name and does not own, load, or unload the supplied library. Lambda operations
and constructors may receive `OpMetadata` as their first recording argument;
the shorter forms use empty metadata.

## Compilation

Compilation lowers mutable resources into versioned values, resolves phi
feedback edges, lowers cutoffs, builds value and resource-order dependencies,
finds strongly connected components, classifies activation and storage,
analyzes lifetimes and interference, assigns slots, allocates aliased storage,
and emits the executable program tables.

`CompilerPlugins` provides ordered `std::function` pass tables after the
normalized, SSA value, producer-dependency, ordered-dependency, compact graph,
storage, lifetime, storage-layout, activation-layout, and final program stages.
Each pass receives the current IR and a `CompileContext` by mutable reference.
Passes for one stage run in insertion order before the next built-in compiler
pass consumes that IR. Typed context data is addressed by a plugin-owned string
ID, is shared across stages, and is retained in the final
`detail::ProgramData` for runtime-specific lowerings. The compiler validates
the IR after every plugin
pass. Invalid ranges, references, layouts, or table sizes fail at the stage
that introduced them. The plugin table and any captured state are externally
synchronized during compilation.

Resource type IDs are preserved from recording through normalized and value IR
into the compiled program. Plugins can therefore recognize resource families
without coupling Flow to those resource types.

The installed `detail::compiler` headers contain the IR records required by
plugin callback signatures. Compiler algorithms, the lowering pipeline, and
runtime implementation live under `src` and are private to the Flow library.

Independent nodes may execute concurrently. Recorded order constrains
operations only where value semantics, resource access ordering, or an
explicit submission boundary requires it.

Loop-invariant code motion moves invariant owned-resource computation before
earlier `acquire` boundaries. Ordinary operations are deterministic with
respect to their declared resource inputs and may affect only their declared
outputs. Operations marked `OpSemantics::side_effect` are not moved. Movable
operations must produce a value, all inputs and ordering predecessors must be
invariant, and every definition of a moved resource must move together. The
analysis propagates through eligible operation chains. It leaves resources
with loop-varying writes inside the activation so concurrent activations retain
independent storage generations. Operations inside structured `if_` and
`while_` regions are not currently moved.

A construction followed by `discard` may be separated from later
loop-varying writes. In that form, construction establishes the C++ object
lifetime while the write after `discard` establishes a fresh logical value.
The compiler hoists the construction and constructs every required physical
storage slot once. Activations lease those already constructed slots after the
discard, and the objects remain alive until program shutdown. Feedback retains
each leased slot until the following activation no longer needs its value.

Owned resources with non-overlapping lifetimes may share storage. A feedback
resource uses two slots when consecutive generations cannot overlap across a
yield. A feedback cycle that crosses a yield reserves up to
`CompileOptions::max_slots` slots. Repeated resources in an acquire-driven
program use the same bound so concurrently live activations receive distinct
storage generations. Slot exhaustion provides backpressure.

The compiler represents each resource lifetime as a half-open interval of
logical schedule positions. These positions are compact `uint32_t` indices and
do not depend on the resource's byte size, alignment, or storage offset. A
construction callback may accept this `flow::Lifetime` immediately after its
destination pointer. The callback receives the same compiled interval whenever
the construction executes. A persistent resource uses the sentinel interval
`[0, UINT32_MAX)` to represent the complete program lifetime.

## Phi, structured control, and cutoff

A phi selects its initial value during its first activation and its retained
feedback value during later activations. The feedback edge is temporal and
does not create an immediate dependency cycle.

`flow::feedback` defines an externally driven cycle. An executable program
containing unconditional feedback must have an acquire before the loop so
that each activation is bounded by a released token. Its operations remain
independent scheduler jobs and retain the parallelism derived by the compiler.

`flow::while_(condition, body)` defines an internally driven pre-test loop.
The recording callback emits the loop body once. At runtime Flow tests the
boolean condition before each iteration and executes the body while it is
true. The body must write the condition, and that value controls the next
iteration. The underscore is part of the API because `while` is a C++
keyword.

`flow::if_(condition, then_body, else_body)` records both branches and executes
only the selected branch at runtime. The underscore is part of the API because
`if` is a C++ keyword. A branch may be empty, but the complete conditional
must contain at least one operation. Conditional construction and discard are
currently rejected because they require branch-sensitive lifetime state.

Every `while_` body and selected `if_` branch executes sequentially inside one
aggregate scheduler job. Nested structured controls run to completion as a
sequential step of their parent. Their intervals must be properly nested or
disjoint. They cannot contain `acquire` or `yield`, because an aggregate job
cannot suspend submission or introduce new scheduler dependencies. A
structured control may be nested inside an acquire-driven region; the
surrounding acquire-driven work retains its normal parallel scheduling.

A cutoff retains a copy of its last observed input and compares each later
input against it. Its output is marked changed on the first evaluation and
whenever the comparison is unequal. Operations downstream of a cutoff are
memoized: they execute when any input value changed and retain their previous
outputs when no input changed. Resource-order edges do not carry value-change
information.

Callbacks must derive their results from their declared resource inputs and
may affect only their declared outputs. Mark an operation with
`OpSemantics::side_effect` when an effect outside its declared resources must
occur. A side-effecting operation is not hoisted or skipped by memoization.

## Submission, acquire, and release

`flow::acquire(name)` records a named counting-semaphore acquisition.
`Program::release(name, count)` adds tokens to that semaphore. Submission
consumes a token when it reaches an acquire. If no token is available,
submission stops before the acquire and returns a completion job for any work
submitted before it. No worker thread blocks while waiting for a token.

`Program::submit(JobScheduler*)` submits executable nodes until the next
unavailable acquire, the next `flow::yield()`, or the end of the current
activation. When an activation completes without reaching a yield, submission
continues into new activations while their acquire tokens are available. A
single call can therefore drain several released tokens. It returns an
aggregate job that completes after every segment and activation submitted by
that call. Calling `submit` again resumes from an unavailable acquire or after
a yield. Reaching the end completes the activation, and the next activation
selects feedback values. Available tokens remain counted across submissions.

An explicit yield always ends the current submission call. The activation and
resources whose lifetimes cross the yield remain alive until a later call
resumes it. The previous submitted segment must complete before the program is
submitted again. The submitting thread may use the returned aggregate job as
a dependency for unrelated or external work before continuing the program.

Activations submitted by one call may execute concurrently. Direct temporal
edges order feedback producers and consumers across consecutive activations.
Once-only nodes are claimed by one activation, and later activations reuse
their results after the claimed producer completes. Cutoff evaluations are
also ordered across activations so that their retained snapshot changes in
activation order.

The compiler reserves up to `CompileOptions::max_slots` activation states for
a program containing acquires. If a memoized constructor replaces a retained
object, one slot remains reserved for that retained generation, so at most
`max_slots - 1` activations overlap. Once the activation states are occupied,
`submit` blocks until a worker completes an activation and returns its state
to the ring. Resource slot exhaustion applies the same backpressure.
Submission does not create worker jobs that wait for activation capacity.

A program without an unavailable acquire or yield submits its complete current
activation. Submitting it again starts another activation. A finite graph is
therefore reusable; the runtime does not permanently enter a finished state.

## Runtime contract

Program submission and external binding are externally synchronized and are
expected to happen on one submitting thread. Worker jobs may execute in
parallel through `JobScheduler`. A program binds to the scheduler used for its
first submission and uses that persistent scheduler for its remaining
lifetime. A submission that produces no new work returns the program's last
completion job.

Recorded callbacks, constructors, equality operations, copy operations, and
destructors are assumed not to throw. Strobe does not provide recovery from
exceptions crossing these operations.

The `Program`, its scheduler, and every bound external object must outlive all
jobs submitted from that program. Destroying or moving a program while its
jobs are outstanding is invalid.

Cutoff snapshots use separate persistent allocations. They normally have long
lifetimes and do not benefit materially from transient lifetime aliasing, so
they are not part of the main aliased resource allocation.

## Render dependency graph integration

Flow remains independent of the RHI. The separate `strobe::rdg` library
translates Flow resource lifetimes into the lifetime locations expected by the
RHI and provides those lifetimes to RHI buffer and image constructors.

An equal cutoff retains the existing strong RHI handle and skips construction.
An unequal cutoff constructs a new handle generation. Destruction remains
implicit: once the old Flow generation has no remaining consumers, destroying
its host-side handle releases Flow's strong reference. The RHI then handles
allocation placement, reference propagation, timeline-based retirement, and
the eventual device-object destruction.

Flow does not model device memory spaces, memory types, queues, or barriers.
RDG compiler plugins use retained resource type IDs and explicit RHI access
scopes to build device barrier plans without adding RHI dependencies to Flow.

## Current scope

Submission uses one global continuation cursor. Consequently, all feedback
components with acquire boundaries advance as part of the same program
activation and cannot suspend and advance at independent rates. Structured
control does not suspend: an `if_` branch or a complete `while_`, including
nested controls, finishes within its aggregate job before dependent jobs run.

The compiler emits a fixed activation-state ring containing value pointers,
argument pointers, current and previous resource generations, and changed-value
flags. Runtime jobs address these tables by activation index. Program
submission and continuation remain externally synchronized even though jobs
from several activations can overlap.

Phi nodes currently support one initial value and one feedback value.
Branch-sensitive construction and destruction and phis with several incoming
control paths are outside the current scope.

Program serialization, symbolic callback registration, and external wake-up
integration are deferred. These do not change the value, lifetime, or
dependency model described above.
