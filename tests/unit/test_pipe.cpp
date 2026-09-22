#include <gtest/gtest.h>
#include <lvtrans/pipe.hpp>
#include <lvtrans/reservoir.hpp>
#include <memory>

TEST(PipeTest, ConnectsToUpstreamReservoir) {
  using namespace lvtrans;
  auto reservoir = std::make_shared<Reservoir>(150.0);
  ASSERT_GT(reservoir->get_ports().size(),
            static_cast<size_t>(PortType::Right));

  auto* reserovoir_port = reservoir->get_ports()[PortType::Right].get();
  ASSERT_NE(reserovoir_port, nullptr);
  EXPECT_EQ(&reserovoir_port->owner, reservoir.get());

  Pipe pipe(
      PipeParameters{
          .length = 600.0,
          .diameter = 0.5,
          .f = 0.018,
          .a = 1200.0,
          .z0 = 10.0,
          .z1 = 15.0,
          .num_reaches = 10,
      },
      150.0, 0.0);
  pipe.connect_to(reservoir.get(), PortType::Left, PortType::Right);

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
