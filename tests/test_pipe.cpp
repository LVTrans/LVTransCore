#include <gtest/gtest.h>
#include <lvtrans/pipe.hpp>
#include <lvtrans/reservoir.hpp>
#include <memory>

TEST(PipeTest, ConnectsToUpstreamReservoir) {
  using namespace lvtrans;
  auto reservoir = std::make_shared<Reservoir>(150.0);
  ASSERT_GT(reservoir->get_ports().size(), static_cast<size_t>(PortRight));

  auto* reserovoir_port = reservoir->get_ports()[PortRight].get();
  ASSERT_NE(reserovoir_port, nullptr);
  EXPECT_EQ(&reserovoir_port->owner, reservoir.get());

  Pipe pipe(PipeConfig{600.0, 0.5, 0.018, 1200.0, 10, 10.0, 15.0}, 150.0, 0.0);
  pipe.connect_left(*reservoir);

  ASSERT_NE(reserovoir_port->connected_to, nullptr);
  EXPECT_EQ(&reserovoir_port->connected_to->owner, &pipe);
  EXPECT_EQ(reserovoir_port->connected_to->connected_to, reserovoir_port);
  EXPECT_NE(pipe.left_elem(), nullptr);
  EXPECT_EQ(pipe.right_elem(), nullptr);

  pipe.iterate();
  EXPECT_DOUBLE_EQ(pipe.get_H().front(), 150.0);
  EXPECT_DOUBLE_EQ(pipe.get_Q().front(), 0.0);

  pipe.remove_left();
  EXPECT_EQ(pipe.left_elem(), nullptr);
  EXPECT_EQ(reserovoir_port->connected_to, nullptr);
}
