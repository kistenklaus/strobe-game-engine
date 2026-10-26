#include <strobe/flow/flow.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace flow = strobe::flow;
using Stream = flow::detail::OpStream;

namespace {

TEST(FlowLifetime, StoresACompactLogicalRange) {
  constexpr flow::Lifetime lifetime{.begin = 7, .end = 19};

  static_assert(sizeof(flow::Lifetime) == sizeof(std::uint32_t) * 2);
  EXPECT_EQ(lifetime.begin, 7);
  EXPECT_EQ(lifetime.end, 19);
}

TEST(FlowVariadicArguments, FlattensResourcesAndPreservesOneLogicalGroup) {
  Stream stream;
  std::array<flow::Resource<int>, 3> resources{
      stream.resource<int>(), stream.resource<int>(), stream.resource<int>()};
  for (std::size_t index = 0; index < resources.size(); ++index) {
    stream.construct(resources[index], static_cast<int>(index));
  }

  stream.op(flow::intrinsic("test", "variadic"),
            flow::read_span(strobe::span<const flow::Resource<int>>{resources}));

  const auto &operation = stream.ops().back();
  ASSERT_EQ(operation.accesses.size(), resources.size());
  ASSERT_EQ(operation.argument_types.size(), resources.size());
  ASSERT_EQ(operation.argument_groups.size(), 1);
  EXPECT_EQ(operation.argument_groups[0].offset, 0);
  EXPECT_EQ(operation.argument_groups[0].count, resources.size());
  for (std::size_t index = 0; index < resources.size(); ++index) {
    EXPECT_EQ(operation.accesses[index].resource, index);
    EXPECT_EQ(operation.accesses[index].mode, flow::AccessMode::read);
  }
}

TEST(FlowVariadicArguments, AllowsAnEmptyFixedArgumentGroup) {
  Stream stream;
  stream.op(flow::intrinsic("test", "empty-variadic"),
            flow::read_span(strobe::span<const flow::Resource<int>>{}));

  const auto &operation = stream.ops().back();
  EXPECT_TRUE(operation.accesses.empty());
  ASSERT_EQ(operation.argument_groups.size(), 1);
  EXPECT_EQ(operation.argument_groups[0].offset, 0);
  EXPECT_EQ(operation.argument_groups[0].count, 0);
}

TEST(FlowOptionalArguments, RecordsPresentAndAbsentResourceGroups) {
  Stream stream;
  auto resource = stream.resource<int>();
  stream.construct(resource, 7);

  stream.op(flow::intrinsic("test", "optional"),
            flow::read(resource), flow::read(flow::Resource<int>{}));

  const auto &operation = stream.ops().back();
  ASSERT_EQ(operation.accesses.size(), 1);
  ASSERT_EQ(operation.argument_groups.size(), 2);
  EXPECT_EQ(operation.argument_groups[0].offset, 0);
  EXPECT_EQ(operation.argument_groups[0].count, 1);
  EXPECT_EQ(operation.argument_groups[1].offset, 1);
  EXPECT_EQ(operation.argument_groups[1].count, 0);
  ASSERT_EQ(operation.argument_elements.size(), 2);
  EXPECT_EQ(operation.argument_elements[0], 0);
  EXPECT_EQ(operation.argument_elements[1], flow::null_argument_element);
}

struct alignas(64) Value {
  int *live;
  int number;

  Value(int *live, int number) : live(live), number(number) { ++*live; }
  Value(const Value &) = delete;
  Value &operator=(const Value &) = delete;
  ~Value() { --*live; }
};

void invoke(Stream &stream, std::size_t index,
            strobe::span<void *const> storage) {
  const_cast<flow::detail::Operation &>(stream.ops()[index])
      .invoke(flow::Lifetime{}, storage);
}

TEST(FlowConstruct, RecordingStoresMetadataWithoutConstructingObjects) {
  int live = 0;
  {
    Stream stream;
    auto value = stream.resource<Value>();
    EXPECT_EQ(stream.size(), 0);
    const auto index = stream.construct(value, &live, 42);
    EXPECT_EQ(live, 0);
    ASSERT_EQ(stream.size(), 1);
    const auto &operation = stream.ops()[index];
    EXPECT_EQ(operation.kind, flow::detail::OperationKind::construct);
    ASSERT_EQ(operation.accesses.size(), 1);
    const auto &info = stream.resources()[operation.accesses[0].resource];
    EXPECT_EQ(info.size, sizeof(Value));
    EXPECT_EQ(info.alignment, alignof(Value));
    EXPECT_EQ(info.construction, index);
    EXPECT_NE(info.destroy, nullptr);
    EXPECT_TRUE(operation.predecessors.empty());
    EXPECT_NO_THROW(stream.validate());
  }
  EXPECT_EQ(live, 0);
}

TEST(FlowConstruct, DeferredCallbacksUseSuppliedStorage) {
  Stream stream;
  int live = 0;
  int observed = 0;
  auto value = stream.resource<Value>();
  stream.construct(value, &live, 42);
  stream.op(flow::read_write(value), [](Value &value) { value.number *= 2; });
  stream.op(flow::read(value),
            [&](const Value &value) { observed = value.number; });
  alignas(Value) std::byte bytes[sizeof(Value)];
  void *storage[] = {bytes};
  invoke(stream, 0, storage);
  EXPECT_EQ(live, 1);
  invoke(stream, 1, storage);
  invoke(stream, 2, storage);
  EXPECT_EQ(observed, 84);
  stream.resources()[stream.ops()[0].accesses[0].resource].destroy(bytes);
  EXPECT_EQ(live, 0);
}

TEST(FlowSerialize, RecordsOrderingWithoutPassingACallbackArgument) {
  Stream stream;
  auto pool = stream.resource<int>();
  auto value = stream.resource<int>();
  stream.construct(pool, 0);
  stream.construct(value, 7);

  int observed = 0;
  const auto operation = stream.op(flow::read(value), flow::serialize(pool),
                                   [&](const int &input) { observed = input; });

  const auto &recorded = stream.ops()[operation];
  ASSERT_EQ(recorded.accesses.size(), 2);
  ASSERT_EQ(recorded.argument_types.size(), 1);
  EXPECT_EQ(recorded.accesses[0].purpose, flow::AccessPurpose::argument);
  EXPECT_EQ(recorded.accesses[1].purpose, flow::AccessPurpose::serialization);
  EXPECT_EQ(recorded.accesses[1].mode, flow::AccessMode::read_write);

  int input = 42;
  void *storage[] = {&input};
  invoke(stream, operation, storage);
  EXPECT_EQ(observed, 42);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowConsume, PassesTheFinalValueAndLeavesTheResourceUndefined) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 7);

  int observed = 0;
  const auto operation = stream.op(flow::consume(value), [&](int &input) {
    observed = input;
    input = 9;
  });
  EXPECT_EQ(stream.ops()[operation].accesses[0].mode,
            flow::AccessMode::consume);

  int input = 7;
  void *storage[] = {&input};
  invoke(stream, operation, storage);
  EXPECT_EQ(observed, 7);
  EXPECT_EQ(input, 9);
  EXPECT_THROW(stream.op(flow::read(value), [](const int &) {}),
               std::logic_error);
}

TEST(FlowConstruct, ConstructsFromRuntimeResourceInputs) {
  Stream stream;
  auto number = stream.resource<int>();
  stream.construct(number, 21);
  auto value = stream.resource<Value>();
  int live = 0;
  const auto construction = stream.construct(
      value, flow::read(number), [&live](Value *storage, const int &number) {
        std::construct_at(storage, &live, number * 2);
      });

  const auto &operation = stream.ops()[construction];
  ASSERT_EQ(operation.accesses.size(), 2);
  EXPECT_EQ(operation.accesses[0].mode, flow::AccessMode::write);
  EXPECT_EQ(operation.accesses[1].mode, flow::AccessMode::read);
  ASSERT_EQ(operation.predecessors.size(), 1);
  EXPECT_EQ(operation.predecessors[0], 0);

  int input = 21;
  alignas(Value) std::byte first[sizeof(Value)];
  void *firstStorage[] = {first, &input};
  invoke(stream, construction, firstStorage);
  EXPECT_EQ(std::launder(reinterpret_cast<Value *>(first))->number, 42);
  stream.resources()[operation.accesses[0].resource].destroy(first);

  input = 7;
  alignas(Value) std::byte second[sizeof(Value)];
  void *secondStorage[] = {second, &input};
  invoke(stream, construction, secondStorage);
  EXPECT_EQ(std::launder(reinterpret_cast<Value *>(second))->number, 14);
  stream.resources()[operation.accesses[0].resource].destroy(second);
  EXPECT_EQ(live, 0);
}

TEST(FlowConstruct, ConstructionPrecedesReadersAndWriters) {
  Stream stream;
  auto a = stream.resource<int>();
  auto b = stream.resource<int>();
  stream.construct(a, 1);
  stream.construct(b, 2);
  stream.op(flow::read(a), [](const int &) {});
  stream.op(flow::read(a), [](const int &) {});
  stream.op(flow::write(a), [](int &a) { a = 4; });
  stream.op(flow::read(a), flow::read(b), [](const int &, const int &) {});
  EXPECT_TRUE(stream.predecessors(1).empty());
  ASSERT_EQ(stream.predecessors(2).size(), 1);
  EXPECT_EQ(stream.predecessors(2)[0], 0);
  ASSERT_EQ(stream.predecessors(3).size(), 1);
  EXPECT_EQ(stream.predecessors(3)[0], 0);
  ASSERT_EQ(stream.predecessors(4).size(), 3);
  EXPECT_EQ(stream.predecessors(4)[0], 0);
  EXPECT_EQ(stream.predecessors(4)[1], 2);
  EXPECT_EQ(stream.predecessors(4)[2], 3);
  ASSERT_EQ(stream.predecessors(5).size(), 3);
  EXPECT_EQ(stream.predecessors(5)[0], 0);
  EXPECT_EQ(stream.predecessors(5)[1], 1);
  EXPECT_EQ(stream.predecessors(5)[2], 4);
}

TEST(FlowConstruct, RejectsMissingDuplicateAndPrematureAccess) {
  Stream stream;
  auto value = stream.resource<int>();
  EXPECT_THROW(stream.validate(), std::logic_error);
  EXPECT_THROW(stream.op(flow::read(value), [](const int &) {}),
               std::logic_error);
  EXPECT_THROW(stream.op(flow::write(value), [](int &) {}), std::logic_error);
  EXPECT_EQ(stream.size(), 0);
  stream.construct(value, 7);
  auto copy = value;
  EXPECT_THROW(stream.construct(copy, 8), std::logic_error);
  EXPECT_EQ(stream.size(), 1);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowConstruct, RejectsDuplicateCallbackAccessesTransactionally) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 7);
  const auto size = stream.size();

  EXPECT_THROW(stream.op(flow::read(value), flow::write(value),
                         [](const int &, int &) {}),
               std::logic_error);
  EXPECT_EQ(stream.size(), size);

  auto output = stream.resource<int>();
  EXPECT_THROW(
      stream.construct(output, flow::read(value), flow::read(value),
                       [](int *output, const int &left, const int &right) {
                         std::construct_at(output, left + right);
                       }),
      std::logic_error);
  EXPECT_EQ(stream.size(), size);
  EXPECT_THROW(stream.validate(), std::logic_error);
  EXPECT_NO_THROW(stream.construct(output, 0));
}

TEST(FlowConstruct, RejectsOutOfRangeResourceIds) {
  Stream first;
  Stream second;
  auto value = first.resource<int>();
  EXPECT_THROW(second.construct(value, 3), std::invalid_argument);
  EXPECT_NO_THROW(first.construct(value, 3));
  EXPECT_THROW(second.op(flow::read(value), [](const int &) {}),
               std::invalid_argument);
  EXPECT_EQ(second.size(), 0);
}

TEST(FlowConstruct, CompleteValidationChecksInternalLinksAndOperations) {
  {
    Stream stream;
    auto value = stream.resource<int>();
    stream.construct(value, 1);
    auto &operation = const_cast<flow::detail::Operation &>(stream.ops()[0]);
    operation.accesses[0].resource = flow::null_resource_id;
    EXPECT_THROW(stream.validate(), std::logic_error);
  }
  {
    Stream stream;
    auto value = stream.resource<int>();
    stream.construct(value, 1);
    auto &operation = const_cast<flow::detail::Operation &>(stream.ops()[0]);
    operation.predecessors.push_back(0);
    EXPECT_THROW(stream.validate(), std::logic_error);
  }
  {
    Stream stream;
    stream.yield();
    auto &operation = const_cast<flow::detail::Operation &>(stream.ops()[0]);
    operation.accesses.push_back({0, flow::AccessMode::read});
    EXPECT_THROW(stream.validate(), std::logic_error);
  }
}

TEST(FlowConstruct, CapturesArgumentsAndSupportsMoveOnlyOwnership) {
  Stream stream;
  auto text = stream.resource<std::string>();
  {
    std::string argument = "retained";
    stream.construct(text, argument);
    argument = "changed";
  }
  auto pointer = stream.resource<std::unique_ptr<int>>();
  stream.construct(pointer, std::make_unique<int>(42));
  alignas(std::string) std::byte textBytes[sizeof(std::string)];
  alignas(std::unique_ptr<int>)
      std::byte pointerBytes[sizeof(std::unique_ptr<int>)];
  void *textStorage[] = {textBytes};
  void *pointerStorage[] = {pointerBytes};
  invoke(stream, 0, textStorage);
  invoke(stream, 1, pointerStorage);
  EXPECT_EQ(*std::launder(reinterpret_cast<std::string *>(textBytes)),
            "retained");
  EXPECT_EQ(
      **std::launder(reinterpret_cast<std::unique_ptr<int> *>(pointerBytes)),
      42);
  stream.resources()[stream.ops()[1].accesses[0].resource].destroy(
      pointerBytes);
  stream.resources()[stream.ops()[0].accesses[0].resource].destroy(textBytes);
}

TEST(FlowConstruct, SupportsNonmovableDefaultConstruction) {
  Stream stream;
  auto mutex = stream.resource<std::mutex>();
  stream.construct(mutex);
  alignas(std::mutex) std::byte bytes[sizeof(std::mutex)];
  void *storage[] = {bytes};
  invoke(stream, 0, storage);
  auto *object = std::launder(reinterpret_cast<std::mutex *>(bytes));
  object->lock();
  object->unlock();
  stream.resources()[stream.ops()[0].accesses[0].resource].destroy(bytes);
}

TEST(FlowConstruct, ConstructorFailureIsDeferredAndDoesNotDestroyAnObject) {
  struct Throws {
    int *destroyed;
    explicit Throws(int *destroyed) : destroyed(destroyed) {
      throw std::runtime_error("constructor failed");
    }
    ~Throws() { ++*destroyed; }
  };
  int destroyed = 0;
  {
    Stream stream;
    auto value = stream.resource<Throws>();
    EXPECT_NO_THROW(stream.construct(value, &destroyed));
    alignas(Throws) std::byte bytes[sizeof(Throws)];
    void *storage[] = {bytes};
    EXPECT_THROW(invoke(stream, 0, storage), std::runtime_error);
  }
  EXPECT_EQ(destroyed, 0);
}

TEST(FlowConstruct, RejectsMissingRuntimeStorage) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 42);
  stream.op(flow::read(value), [](const int &) {});
  void *nullStorage[] = {nullptr};
  EXPECT_THROW(invoke(stream, 0, {}), std::invalid_argument);
  EXPECT_THROW(invoke(stream, 0, nullStorage), std::invalid_argument);
  EXPECT_THROW(invoke(stream, 1, {}), std::invalid_argument);
  EXPECT_THROW(invoke(stream, 1, nullStorage), std::invalid_argument);
  int calls = 0;
  stream.op([&] { ++calls; });
  invoke(stream, 2, {});
  EXPECT_EQ(calls, 1);
}

TEST(FlowConstruct, EndInvalidatesRecordingAndRecordingRemainsThreadLocal) {
  flow::begin();
  auto value = flow::resource<int>();
  EXPECT_THROW(flow::end(), std::logic_error);
  EXPECT_THROW(flow::construct(value, 42), std::logic_error);

  std::thread worker([&] {
    flow::begin();
    EXPECT_THROW(flow::construct(value, 0), std::invalid_argument);
    flow::end();
    EXPECT_TRUE(flow::ops().empty());
  });
  worker.join();

  flow::begin();
  EXPECT_THROW(flow::construct(value, 0), std::invalid_argument);
  auto replacement = flow::resource<int>();
  flow::construct(replacement, 42);
  flow::end();
  EXPECT_EQ(flow::ops().size(), 1);
}

TEST(FlowYield, RecordsAZeroAccessSubmissionBoundary) {
  Stream stream;
  const auto first = stream.yield();
  const auto second = stream.yield();

  ASSERT_EQ(stream.ops().size(), 2);
  EXPECT_EQ(first, 0);
  EXPECT_EQ(second, 1);
  for (const auto &operation : stream.ops()) {
    EXPECT_EQ(operation.kind, flow::detail::OperationKind::yield);
    EXPECT_TRUE(operation.accesses.empty());
    EXPECT_TRUE(operation.predecessors.empty());
  }
  EXPECT_NO_THROW(invoke(stream, first, {}));
  EXPECT_NO_THROW(invoke(stream, second, {}));
}

TEST(FlowYield, GlobalFunctionRequiresAnActiveRecording) {
  flow::begin();
  const auto index = flow::yield();
  flow::end();

  ASSERT_EQ(flow::ops().size(), 1);
  EXPECT_EQ(index, 0);
  EXPECT_EQ(flow::ops()[0].kind, flow::detail::OperationKind::yield);
  EXPECT_THROW(flow::yield(), std::logic_error);
}

TEST(FlowAcquire, RecordsNamedCountingSemaphoreAcquisitions) {
  Stream stream;
  const auto first = stream.acquire("frame");
  const auto second = stream.acquire("frame");

  ASSERT_EQ(stream.ops().size(), 2);
  ASSERT_EQ(stream.acquires().size(), 2);
  EXPECT_EQ(first, 0);
  EXPECT_EQ(second, 1);
  EXPECT_EQ(stream.ops()[first].kind, flow::detail::OperationKind::acquire);
  EXPECT_EQ(stream.acquires()[0].name.view(), "frame");
  EXPECT_EQ(stream.acquires()[1].name.view(), "frame");
  EXPECT_THROW(stream.acquire(""), std::invalid_argument);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowAcquire, GlobalFunctionRequiresAnActiveRecording) {
  flow::begin();
  const auto index = flow::acquire("frame");
  flow::end();

  EXPECT_EQ(index, 0);
  EXPECT_EQ(flow::ops()[0].kind, flow::detail::OperationKind::acquire);
  EXPECT_THROW(flow::acquire("frame"), std::logic_error);
}

TEST(FlowDiscard, InvalidatesAndWriteReestablishesTheLogicalValue) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 42);
  const auto discard = stream.discard(value);

  ASSERT_EQ(stream.ops()[discard].kind, flow::detail::OperationKind::discard);
  ASSERT_EQ(stream.ops()[discard].accesses.size(), 1);
  EXPECT_EQ(stream.ops()[discard].accesses[0].mode, flow::AccessMode::write);
  EXPECT_NO_THROW(invoke(stream, discard, {}));
  EXPECT_THROW(stream.op(flow::read(value), [](const int &) {}),
               std::logic_error);
  EXPECT_THROW(stream.op(flow::read_write(value), [](int &) {}),
               std::logic_error);
  EXPECT_THROW(stream.discard(value), std::logic_error);

  EXPECT_NO_THROW(stream.op(flow::write(value), [](int &value) { value = 7; }));
  EXPECT_NO_THROW(stream.op(flow::read(value), [](const int &) {}));
}

TEST(FlowDiscard, GlobalFunctionRecordsDiscard) {
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, 42);
  const auto index = flow::discard(value);
  flow::op(flow::write(value), [](int &value) { value = 0; });
  flow::end();

  EXPECT_EQ(flow::ops()[index].kind, flow::detail::OperationKind::discard);
}

TEST(FlowIntrinsic, RecordsStableIdentityAndResourceAccesses) {
  Stream stream;
  auto input = stream.external<int>("input");
  auto output = stream.resource<int>();
  stream.construct(output, 0);

  const auto index = stream.op(flow::intrinsic("strobe.test", "copy", 3),
                               flow::read(input), flow::write(output));

  ASSERT_TRUE(stream.ops()[index].intrinsic.has_value());
  EXPECT_EQ(stream.ops()[index].intrinsic->domain.view(), "strobe.test");
  EXPECT_EQ(stream.ops()[index].intrinsic->opcode.view(), "copy");
  EXPECT_EQ(stream.ops()[index].intrinsic->version, 3);
  ASSERT_EQ(stream.ops()[index].accesses.size(), 2);
  EXPECT_EQ(stream.ops()[index].accesses[0].mode, flow::AccessMode::read);
  EXPECT_EQ(stream.ops()[index].accesses[1].mode, flow::AccessMode::write);
  EXPECT_FALSE(stream.ops()[index].invoke);
  EXPECT_FALSE(stream.ops()[index].external.has_value());
  EXPECT_NO_THROW(stream.validate());

  EXPECT_THROW(flow::intrinsic("", "copy"), std::invalid_argument);
  EXPECT_THROW(flow::intrinsic("strobe.test", ""), std::invalid_argument);
  EXPECT_THROW(flow::intrinsic("strobe.test", "copy", 0),
               std::invalid_argument);
}

TEST(FlowIntrinsic, StoresTypedTrivialImmediates) {
  enum class Stage : std::uint32_t { transfer = 7 };
  struct Extent {
    std::uint32_t width;
    std::uint32_t height;
  };
  static_assert(std::is_trivially_copyable_v<Extent>);

  auto source = flow::intrinsic("strobe.test", "immediates", 2)
                    .immediate(Stage::transfer)
                    .immediate(Extent{.width = 1280, .height = 720});
  ASSERT_EQ(source.immediates.size(), 2);
  EXPECT_EQ(std::as_const(source).immediate<Stage>(0), Stage::transfer);
  const auto extent = std::as_const(source).immediate<Extent>(1);
  EXPECT_EQ(extent.width, 1280);
  EXPECT_EQ(extent.height, 720);
  EXPECT_THROW(std::as_const(source).immediate<std::uint32_t>(0),
               std::invalid_argument);
  EXPECT_THROW(std::as_const(source).immediate<Stage>(2), std::out_of_range);

  Stream stream;
  stream.op(std::move(source));
  stream.validate();
  ASSERT_TRUE(stream.ops()[0].intrinsic);
  EXPECT_EQ(stream.ops()[0].intrinsic->immediate<Extent>(1).height, 720);
}

TEST(FlowIntrinsic, StoresTypedTrivialImmediateSpans) {
  const std::array<std::uint32_t, 3> values{4, 8, 15};
  auto source = flow::intrinsic("strobe.test", "immediate-span")
                    .immediate_span(strobe::span<const std::uint32_t>{values});

  ASSERT_EQ(source.immediates.size(), 1);
  EXPECT_EQ(source.immediates[0].type, flow::type_id<std::uint32_t>());
  EXPECT_EQ(source.immediates[0].element_count, values.size());
  EXPECT_EQ(source.immediates[0].element_size, sizeof(std::uint32_t));
  EXPECT_EQ(source.immediates[0].size, sizeof(values));
  EXPECT_EQ(source.immediate_data.size(), sizeof(values));
  EXPECT_EQ(
      std::memcmp(source.immediate_data.data(), values.data(), sizeof(values)),
      0);
  EXPECT_THROW(std::as_const(source).immediate<std::uint32_t>(0),
               std::invalid_argument);

  const strobe::span<const std::uint32_t> empty;
  source.immediate_span(empty);
  EXPECT_EQ(source.immediates[1].element_count, 0);
  EXPECT_EQ(source.immediates[1].size, 0);

  Stream stream;
  stream.op(std::move(source));
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowOperationFormat, PrintsSourceAndAccesses) {
  Stream stream;
  auto input = stream.external<int>("input");
  auto output = stream.resource<int>();
  stream.construct(output, 0);
  const auto index = stream.op(flow::intrinsic("strobe.test", "copy", 3),
                               flow::read(input), flow::write(output));

  EXPECT_EQ(fmt::format("{}", stream.ops()[index]),
            fmt::format("{:<50}(strobe.test/copy)", "op r:%0, w:%1"));
}

TEST(FlowOperationFormat, PrintsOperationComponents) {
  EXPECT_EQ(fmt::format("{}", flow::AccessMode::read_write), "rw");
  EXPECT_EQ(fmt::format("{}", flow::AccessMode::consume), "c");
  EXPECT_EQ(fmt::format("{}", flow::detail::OperationKind::construct),
            "construct");
  EXPECT_EQ(fmt::format("{}",
                        flow::detail::AccessInfo{
                            .resource = 7, .mode = flow::AccessMode::write}),
            "w:%7");
  EXPECT_EQ(fmt::format("{}", flow::OpMetadata("upload", "uploads data",
                                               flow::OpSemantics::side_effect)),
            "[\"upload\", side_effect]");
  EXPECT_EQ(fmt::format("{}", flow::OpMetadata("upload")), "[\"upload\"]");
  EXPECT_EQ(fmt::format("{}", flow::intrinsic("strobe.test", "copy", 3)),
            "strobe.test/copy");
  EXPECT_EQ(fmt::format("{}", flow::external_op("renderer", "draw", 2)),
            "renderer::draw");
  EXPECT_EQ(fmt::format("{}", flow::TypeId{0x2a}), "type(0x000000000000002a)");

  flow::detail::Operation operation;
  operation.metadata =
      flow::OpMetadata("upload", "not printed", flow::OpSemantics::side_effect);
  EXPECT_EQ(fmt::format("{}", operation),
            fmt::format("{:<50}(<lambda>) [\"upload\", side_effect]", "op"));

  operation.metadata = flow::OpMetadata("", "diagnostic description");
  EXPECT_EQ(fmt::format("{}", operation),
            fmt::format("{:<50}(<lambda>)", "op"));
}

TEST(FlowOperationFormat, PrettyPrintsOneSourceOperationPerLine) {
  Stream stream;
  auto input = stream.external<int>("input");
  auto output = stream.resource<int>();
  stream.construct(output, 0);
  stream.op(flow::intrinsic("strobe.test", "copy", 3), flow::read(input),
            flow::write(output));

  const auto expected =
      fmt::format("%0 = external \"input\" : {}\n{:<50}(<lambda>)\n"
                  "{:<50}(strobe.test/copy)",
                  flow::type_id<int>(), "%1 = construct", "op r:%0, w:%1");
  EXPECT_EQ(flow::detail::to_pretty(stream).view(), expected);
}

TEST(FlowOperationFormat, PrettyPrintsPhiAndFeedbackAtTheirBoundaries) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.feedback(state, next, flow::FeedbackOptions{.max_in_flight = 3});

  const auto expected =
      fmt::format("{:<50}(<lambda>)\n%1 = phi %0\n{:<50}(<lambda>)\n"
                  "{:<50}[max_in_flight=3]",
                  "%0 = construct", "%2 = construct", "feedback %1 <- %2");
  EXPECT_EQ(flow::detail::to_pretty(stream).view(), expected);
}

TEST(FlowOperationFormat, PrettyPrintsAcquireTokenNames) {
  Stream stream;
  stream.acquire("frame");

  EXPECT_EQ(flow::detail::to_pretty(stream).view(),
            fmt::format("{:<50}(<lambda>)", "acquire \"frame\""));
}

TEST(FlowIntrinsic, RecordsConstructionWithAnImplicitOutput) {
  Stream stream;
  auto input = stream.external<int>("input");
  auto output = stream.resource<float>();

  const auto index =
      stream.construct(output, flow::intrinsic("strobe.test", "construct", 2),
                       flow::read(input));

  const auto &operation = stream.ops()[index];
  EXPECT_EQ(operation.kind, flow::detail::OperationKind::construct);
  ASSERT_TRUE(operation.intrinsic.has_value());
  EXPECT_EQ(operation.intrinsic->domain.view(), "strobe.test");
  EXPECT_EQ(operation.intrinsic->opcode.view(), "construct");
  EXPECT_EQ(operation.intrinsic->version, 2);
  ASSERT_EQ(operation.accesses.size(), 2);
  EXPECT_EQ(operation.accesses[0].mode, flow::AccessMode::write);
  EXPECT_EQ(operation.accesses[1].mode, flow::AccessMode::read);
  EXPECT_EQ(operation.argument_types[0], flow::type_id<float>());
  EXPECT_EQ(operation.argument_types[1], flow::type_id<int>());
  EXPECT_FALSE(operation.invoke);
  EXPECT_NO_THROW(stream.op(flow::read(output), [](const float &) {}));
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowCutoff, RecordsVirtualOutputAndEquality) {
  Stream stream;
  auto input = stream.resource<int>();
  stream.construct(input, 42);
  auto stable = stream.cutoff(input);
  stream.op(flow::read(stable), [](const int &) {});

  ASSERT_EQ(stream.cutoffs().size(), 1);
  const auto &cutoff = stream.cutoffs()[0];
  EXPECT_EQ(stream.ops()[cutoff.operation].kind,
            flow::detail::OperationKind::cutoff);
  EXPECT_EQ(stream.resources()[cutoff.output].cutoff, 0);
  EXPECT_EQ(stream.resources()[cutoff.output].size, 0);
  EXPECT_EQ(stream.resources()[cutoff.output].destroy, nullptr);
  const int equal = 42;
  const int different = 7;
  EXPECT_TRUE(cutoff.equal(&equal, &equal));
  EXPECT_FALSE(cutoff.equal(&equal, &different));
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowCutoff, RequiresADefinedEqualityComparableInput) {
  Stream stream;
  auto value = stream.resource<int>();
  EXPECT_THROW(stream.cutoff(value), std::logic_error);
  stream.construct(value, 1);
  stream.discard(value);
  EXPECT_THROW(stream.cutoff(value), std::logic_error);
}

TEST(FlowCutoff, GlobalFunctionRecordsCutoff) {
  flow::begin();
  auto input = flow::resource<int>();
  flow::construct(input, 42);
  auto stable = flow::cutoff(input);
  flow::op(flow::read(stable), [](const int &) {});
  flow::end();

  EXPECT_EQ(flow::ops()[1].kind, flow::detail::OperationKind::cutoff);
}

TEST(FlowExternal, RecordsNamedBorrowedResourceMetadata) {
  Stream stream;
  auto window = stream.external<int>("window");
  stream.op(flow::read(window), [](const int &) {});
  stream.op(flow::write(window), [](int &) {});

  ASSERT_EQ(stream.externals().size(), 1);
  const auto &external = stream.externals()[0];
  EXPECT_EQ(external.name.view(), "window");
  const auto &resource = stream.resources()[external.resource];
  EXPECT_EQ(resource.external, 0);
  EXPECT_EQ(resource.size, sizeof(int));
  EXPECT_EQ(resource.alignment, alignof(int));
  EXPECT_EQ(resource.destroy, nullptr);
  EXPECT_NO_THROW(stream.validate());
  EXPECT_THROW(stream.construct(window, 1), std::logic_error);
  EXPECT_THROW(stream.discard(window), std::logic_error);
}

TEST(FlowExternal, InternsMatchingNamesAndRejectsTypeMismatches) {
  Stream stream;
  EXPECT_THROW(stream.external<int>(""), std::invalid_argument);
  const auto first = stream.external<int>("window");
  const auto second = stream.external<int>("window");
  stream.op(flow::read(first), [](const int &) {});
  stream.op(flow::read(second), [](const int &) {});

  ASSERT_EQ(stream.externals().size(), 1);
  ASSERT_EQ(stream.resources().size(), 1);
  ASSERT_EQ(stream.ops().size(), 2);
  EXPECT_EQ(stream.ops()[0].accesses[0].resource,
            stream.ops()[1].accesses[0].resource);
  EXPECT_THROW(stream.external<float>("window"), std::invalid_argument);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowExternal, GlobalFunctionRecordsExternal) {
  flow::begin();
  auto window = flow::external<int>("window");
  flow::op(flow::read(window), [](const int &) {});
  EXPECT_NO_THROW(flow::end());
}

TEST(FlowPhi, RecordsInitialAndFeedbackResources) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 0);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);

  ASSERT_EQ(stream.phis().size(), 1);
  const auto &phi = stream.phis()[0];
  EXPECT_EQ(phi.initial, stream.ops()[0].accesses[0].resource);
  ASSERT_TRUE(phi.feedback);
  EXPECT_EQ(*phi.feedback, stream.ops()[1].accesses[0].resource);
  EXPECT_EQ(stream.resources()[phi.output].phi, 0);
  EXPECT_EQ(stream.resources()[phi.output].size, 0);
  EXPECT_EQ(stream.resources()[phi.output].destroy, nullptr);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowPhi, ValidatesDefinitionsAndExactlyOneFeedback) {
  Stream stream;
  auto initial = stream.resource<int>();
  EXPECT_THROW(stream.phi(initial), std::logic_error);
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  EXPECT_NO_THROW(stream.op(flow::read(state), [](const int &) {}));
  EXPECT_THROW(stream.construct(state, 1), std::logic_error);
  EXPECT_THROW(stream.validate(), std::logic_error);

  auto next = stream.resource<int>();
  EXPECT_THROW(stream.feedback(state, next), std::logic_error);
  stream.construct(next, 1);
  EXPECT_THROW(stream.feedback(initial, next), std::invalid_argument);
  EXPECT_NO_THROW(stream.feedback(state, next));
  EXPECT_THROW(stream.feedback(state, next), std::logic_error);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowPhi, GlobalApiRecordsPhiRelation) {
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<int>();
  flow::construct(next, 0);
  flow::op(flow::read(state), flow::write(next),
           [](const int &state, int &next) { next = state + 1; });
  flow::feedback(state, next);
  EXPECT_NO_THROW(flow::end());
}

TEST(FlowPhi, RejectsZeroFeedbackInFlightLimit) {
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<int>();
  flow::construct(next, 1);
  EXPECT_THROW(
      flow::feedback(state, next, flow::FeedbackOptions{.max_in_flight = 0}),
      std::invalid_argument);
  flow::feedback(state, next);
  EXPECT_NO_THROW((void)flow::end());
}

TEST(FlowWhile, RecordsAHiddenConditionalBackedge) {
  Stream stream;
  auto condition = stream.resource<bool>();
  stream.construct(condition, true);
  stream.while_(condition, [&] {
    stream.op(flow::write(condition), [](bool &repeat) { repeat = false; });
  });

  ASSERT_EQ(stream.phis().size(), 1);
  ASSERT_TRUE(stream.phis()[0].condition);
  EXPECT_EQ(*stream.phis()[0].condition, stream.ops()[1].accesses[0].resource);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowIf, RecordsBothBranchIntervals) {
  Stream stream;
  auto condition = stream.resource<bool>();
  stream.construct(condition, true);
  auto value = stream.resource<int>();
  stream.construct(value, 0);
  stream.if_(
      condition,
      [&] { stream.op(flow::write(value), [](int &output) { output = 1; }); },
      [&] { stream.op(flow::write(value), [](int &output) { output = 2; }); });

  ASSERT_EQ(stream.ifs().size(), 1);
  EXPECT_EQ(stream.ifs()[0].condition, stream.ops()[0].accesses[0].resource);
  EXPECT_EQ(stream.ifs()[0].begin, 2);
  EXPECT_EQ(stream.ifs()[0].alternative, 3);
  EXPECT_EQ(stream.ifs()[0].end, 4);
  EXPECT_NO_THROW(stream.validate());
}

TEST(FlowIf, RejectsSubmissionBoundariesAndConditionalLifetimeChanges) {
  {
    Stream stream;
    auto condition = stream.resource<bool>();
    stream.construct(condition, true);
    EXPECT_THROW(stream.if_(
                     condition, [&] { stream.yield(); }, [] {}),
                 std::logic_error);
  }
  {
    Stream stream;
    auto condition = stream.resource<bool>();
    stream.construct(condition, true);
    auto value = stream.resource<int>();
    EXPECT_THROW(stream.if_(
                     condition, [&] { stream.construct(value, 1); }, [] {}),
                 std::logic_error);
  }
}

TEST(FlowIf, RejectsConsumeInEitherConditionalBranch) {
  {
    Stream stream;
    auto condition = stream.resource<bool>();
    auto value = stream.resource<int>();
    stream.construct(condition, true);
    stream.construct(value, 1);
    EXPECT_THROW(stream.if_(
                     condition,
                     [&] { stream.op(flow::consume(value), [](int &) {}); },
                     [] {}),
                 std::logic_error);
  }

  {
    Stream stream;
    auto condition = stream.resource<bool>();
    auto value = stream.resource<int>();
    stream.construct(condition, true);
    stream.construct(value, 1);
    EXPECT_THROW(stream.if_(
                     condition, [] {},
                     [&] { stream.op(flow::consume(value), [](int &) {}); }),
                 std::logic_error);
  }
}

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
