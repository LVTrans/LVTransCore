#include <gtest/gtest.h>
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/t_1in_2out_n.hpp"
#include "lvtrans/hydraulic_connections.hpp"

using namespace lvtrans;
TEST(T_junction, T1In2OutN) {
  Pipe pipe1(0.1);
  Pipe pipe2(0.1);
  Pipe pipe3(0.1);
  T1In2OutN t_junction(TConnectionParameters{});

  EXPECT_EQ(t_junction.left_links().size(), 1);
  EXPECT_EQ(t_junction.right_links().size(), 2);

  EXPECT_EQ(HydraulicConnections::connect(pipe1, PipeEnd::Right, t_junction),
            ConnectResult::Ok);
  EXPECT_EQ(HydraulicConnections::connect(pipe2, PipeEnd::Left, t_junction, 0),
            ConnectResult::Ok);
  EXPECT_EQ(HydraulicConnections::connect(pipe3, PipeEnd::Left, t_junction, 1),
            ConnectResult::Ok);

  EXPECT_EQ(t_junction.left_links()[0].pipe, &pipe1);
  EXPECT_EQ(t_junction.right_links()[0].pipe, &pipe2);
  EXPECT_EQ(t_junction.right_links()[1].pipe, &pipe3);
}

TEST(T_junction, SimpleConnection) {
  Pipe pipe1(0.1);
  Pipe pipe2(0.1);
  Pipe pipe3(0.1);
  SimpleConnection simple_connection{};
  EXPECT_EQ(simple_connection.left_links().size(), 1);
  EXPECT_EQ(simple_connection.right_links().size(), 1);
  EXPECT_EQ(
      HydraulicConnections::connect(pipe1, PipeEnd::Right, simple_connection),
      ConnectResult::Ok);
  EXPECT_EQ(
      HydraulicConnections::connect(pipe2, PipeEnd::Left, simple_connection),
      ConnectResult::Ok);
  EXPECT_EQ(simple_connection.left_links()[0].pipe, &pipe1);
  EXPECT_EQ(simple_connection.right_links()[0].pipe, &pipe2);
}
