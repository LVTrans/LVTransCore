#include "lvtrans/hydraulic_connections.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

ConnectResult HydraulicConnections::connect(BasePipe& pipe, PipeEnd end,
                                            NonPipe& element,
                                            std::size_t terminal_index) {
  const auto side = end == PipeEnd::Left ? Side::Right : Side::Left;
  auto& slots = side == Side::Left ? element.m_left : element.m_right;

  if (terminal_index >= slots.size()) {
    return ConnectResult::InvalidTerminal;
  }

  auto& boundary = end == PipeEnd::Left ? pipe.m_left : pipe.m_right;

  if (boundary.element || slots[terminal_index].pipe) {
    return ConnectResult::AlreadyConnected;
  }

  boundary = {&element, {side, terminal_index}};
  slots[terminal_index] = {&pipe, end};

  return ConnectResult::Ok;
}

void HydraulicConnections::disconnect(BasePipe& pipe, PipeEnd end) {
  auto& boundary = end == PipeEnd::Left ? pipe.m_left : pipe.m_right;
  if (!boundary.element) return;

  auto& slots = boundary.terminal.side == Side::Left
                    ? boundary.element->m_left
                    : boundary.element->m_right;
  slots[boundary.terminal.index] = {};
  boundary = {};
}

void HydraulicConnections::disconnect_all(BasePipe& pipe) {
  disconnect(pipe, PipeEnd::Left);
  disconnect(pipe, PipeEnd::Right);
}

void HydraulicConnections::disconnect_all(NonPipe& element) {
  for (auto& link : element.m_left)
    if (link.pipe) {
      disconnect(*link.pipe, link.end);
    }
  for (auto& link : element.m_right)
    if (link.pipe) {
      disconnect(*link.pipe, link.end);
    }
}
}  // namespace lvtrans
