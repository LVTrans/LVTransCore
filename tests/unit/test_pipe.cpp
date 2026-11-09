#include <gtest/gtest.h>
#include <lvtrans/elements/pipe.hpp>
#include <lvtrans/elements/reservoir.hpp>
#include <memory>
#include "lvtrans/elements/constant_level_left.hpp"

TEST(PipeTest, CharacteristicsUseGridAdjustedWaveSpeed) {
  using namespace lvtrans;
  PipeParameters parameters{
      .length = 100.0, .diameter = 1.0, .a = 1200.0, .lambda = 0.0};
  const auto no_damping = calculate_pipe_data(parameters, 0.1);
  EXPECT_EQ(no_damping.grid.num_segments, 2u);
  EXPECT_DOUBLE_EQ(no_damping.grid.dx, 50.0);
  EXPECT_DOUBLE_EQ(no_damping.grid.wave_speed, 1000.0);
  // Original pipe_core: B = adjusted_a / (area * 9.82).
  EXPECT_NEAR(no_damping.coefficients.B, 129.65779477954814, 1e-10);

  parameters.lambda = 500000.0;
  const auto damped = calculate_pipe_data(parameters, 0.1);
  EXPECT_NEAR(damped.grid.wave_speed, 1004.987562112089, 1e-10);
  EXPECT_NEAR(damped.coefficients.B, 130.30447108432762, 1e-10);
}

TEST(PipeTest, ReadViewExportsEvenNodesWithMatchingPhysicalCoordinates) {
  using namespace lvtrans;
  const std::vector<double> heads{100.0, 95.0, 90.0};
  const std::vector<double> flows{1.0, 2.0, 3.0};
  Pipe pipe(PipeParameters{.H0 = heads,
                           .Q0 = flows,
                           .length = 100.0,
                           .a = 1200.0,
                           .lambda = 500e3},
            0.1);

  const auto view = pipe.read_view();
  ASSERT_EQ(view.values.size(), 2u);
  const auto& head = std::get<VectorValue>(view.values[0]);
  const auto& flow = std::get<VectorValue>(view.values[1]);
  const std::vector<double> expected_x{0.0, 100.0};
  EXPECT_EQ(head.x_values, expected_x);
  EXPECT_EQ(flow.x_values, expected_x);
  EXPECT_EQ(head.values, (std::vector<double>{100.0, 90.0}));
  EXPECT_EQ(flow.values, (std::vector<double>{1.0, 3.0}));
  EXPECT_EQ(pipe.get_H(), heads);
  EXPECT_EQ(pipe.get_Q(), flows);
  EXPECT_EQ(head.x_unit, "m");
  EXPECT_EQ(flow.x_unit, "m");
}

TEST(PipeTest, ConnectsToUpstreamReservoir) {
  using namespace lvtrans;
  auto reservoir = std::make_shared<ConstantLevelLeft>(150.0);
  ASSERT_EQ(reservoir->right_links().size(), 1u);
  EXPECT_EQ(reservoir->right(), nullptr);

  Pipe pipe(PipeParameters{.H0 = 150.0,
                           .Q0 = 0.0,
                           .length = 600.0,
                           .diameter = 0.5,
                           .f = 0.018,
                           .a = 1200.0,
                           .z0 = 10.0,
                           .z1 = 15.0,
                           .lambda = 0},
            0.1);
  ASSERT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, *reservoir),
            ConnectResult::Ok);

  EXPECT_EQ(reservoir->right(), &pipe);
  EXPECT_NE(pipe.left_elem(), nullptr);
  EXPECT_EQ(pipe.right_elem(), nullptr);

  pipe.iterate();
  EXPECT_DOUBLE_EQ(pipe.get_H().front(), 150.0);
  EXPECT_DOUBLE_EQ(pipe.get_Q().front(), 0.0);

  HydraulicConnections::disconnect(pipe, PipeEnd::Left);
  EXPECT_EQ(pipe.left_elem(), nullptr);
  EXPECT_EQ(reservoir->right(), nullptr);
}

TEST(PipeTest, InvalidOrOccupiedSlotsLeaveExistingConnectionsIntact) {
  using namespace lvtrans;
  ConstantLevelLeft first(150.0), second(140.0);
  Pipe pipe(PipeParameters{}, 0.1), other(PipeParameters{}, 0.1);

  EXPECT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Right, first),
            ConnectResult::InvalidTerminal);
  EXPECT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, first, 1),
            ConnectResult::InvalidTerminal);
  EXPECT_EQ(pipe.left_elem(), nullptr);
  EXPECT_EQ(pipe.right_elem(), nullptr);
  EXPECT_EQ(first.right_links()[0].pipe, nullptr);

  ASSERT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, first),
            ConnectResult::Ok);
  EXPECT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, second),
            ConnectResult::AlreadyConnected);
  EXPECT_EQ(HydraulicConnections::connect(other, PipeEnd::Left, first),
            ConnectResult::AlreadyConnected);
  EXPECT_EQ(pipe.left_elem(), &first);
  EXPECT_EQ(first.right_links()[0].pipe, &pipe);
  EXPECT_EQ(second.right_links()[0].pipe, nullptr);
  EXPECT_EQ(other.left_elem(), nullptr);
}

TEST(PipeTest, DestroyingEitherEndpointDisconnectsTheSurvivor) {
  using namespace lvtrans;
  ConstantLevelLeft reservoir(150.0);
  {
    Pipe pipe(PipeParameters{}, 0.1);
    ASSERT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, reservoir),
              ConnectResult::Ok);
  }
  EXPECT_EQ(reservoir.right_links()[0].pipe, nullptr);

  Pipe pipe(PipeParameters{}, 0.1);
  {
    ConstantLevelLeft temporary(150.0);
    ASSERT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, temporary),
              ConnectResult::Ok);
  }
  EXPECT_EQ(pipe.left_elem(), nullptr);
  EXPECT_NO_THROW(pipe.disconnect_all());
}

namespace {
class ReservoirSlots : public lvtrans::Reservoir {
 public:
  using Reservoir::Reservoir;
  double get_H(lvtrans::CharacteristicsType, lvtrans::ElementID) override {
    return 0.0;
  }
  double get_Q(lvtrans::CharacteristicsType, lvtrans::ElementID) override {
    return 0.0;
  }
};
}  // namespace

TEST(HydraulicConnectionsTest, IndexedLinksRemainStableWhenDisconnected) {
  using namespace lvtrans;
  ReservoirSlots reservoir(150.0, 2, 2);
  Pipe first(PipeParameters{}, 0.1), second(PipeParameters{}, 0.1);
  ASSERT_EQ(HydraulicConnections::connect(first, PipeEnd::Left, reservoir, 1),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(second, PipeEnd::Left, reservoir, 0),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(first, PipeEnd::Right, reservoir, 1),
            ConnectResult::Ok);
  EXPECT_EQ(first.left_link().terminal, (TerminalId{Side::Right, 1}));
  EXPECT_EQ(first.right_link().terminal, (TerminalId{Side::Left, 1}));
  EXPECT_EQ(reservoir.right_links()[1].pipe, &first);
  EXPECT_EQ(reservoir.right_links()[1].end, PipeEnd::Left);
  EXPECT_EQ(reservoir.left_links()[1].end, PipeEnd::Right);

  HydraulicConnections::disconnect(second, PipeEnd::Left);
  EXPECT_EQ(reservoir.right_links().size(), 2u);
  EXPECT_EQ(reservoir.right_links()[0].pipe, nullptr);
  EXPECT_EQ(reservoir.right_links()[1].pipe, &first);
  EXPECT_EQ(first.left_link().terminal.index, 1u);

  HydraulicConnections::disconnect_all(reservoir);
  EXPECT_EQ(first.left_elem(), nullptr);
  EXPECT_EQ(first.right_elem(), nullptr);
  EXPECT_EQ(reservoir.left_links()[1].pipe, nullptr);
  EXPECT_EQ(reservoir.right_links()[1].pipe, nullptr);
  EXPECT_NO_THROW(HydraulicConnections::disconnect_all(reservoir));
}

TEST(HydraulicConnectionsTest, InvalidPipeEndDoesNotChangeConnections) {
  using namespace lvtrans;
  ConstantLevelLeft reservoir(150.0);
  Pipe pipe(PipeParameters{}, 0.1);
  ASSERT_EQ(HydraulicConnections::connect(pipe, PipeEnd::Left, reservoir),
            ConnectResult::Ok);
  const auto invalid = static_cast<PipeEnd>(99);
  EXPECT_EQ(HydraulicConnections::connect(pipe, invalid, reservoir),
            ConnectResult::InvalidTerminal);
  HydraulicConnections::disconnect(pipe, invalid);
  EXPECT_EQ(pipe.left_elem(), &reservoir);
  EXPECT_EQ(reservoir.right_links()[0].pipe, &pipe);
  HydraulicConnections::disconnect_all(pipe);
  EXPECT_EQ(reservoir.right_links()[0].pipe, nullptr);
}
