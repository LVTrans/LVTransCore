#include "lvtrans/hydraulic_connections.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

ConnectResult HydraulicConnections::connect(BasePipe& pipe, const PipeEnd end,
                                            NonPipe& element,
                                            const std::size_t terminal_index) {
  const auto side = end == PipeEnd::Left ? Side::Right : Side::Left;
  auto& slots = side == Side::Left ? element.m_left : element.m_right;

  if (terminal_index >= slots.size()) {
    return ConnectResult::InvalidTerminal;
  }

  auto& boundary = end == PipeEnd::Left ? pipe.m_left : pipe.m_right;

  if (boundary.element || slots[terminal_index].pipe) {
    return ConnectResult::AlreadyConnected;
  }

  boundary = {.element = &element,
              .terminal = {.side = side, .index = terminal_index}};
  slots[terminal_index] = {.pipe = &pipe, .end = end};

  return ConnectResult::Ok;
}

void HydraulicConnections::disconnect(BasePipe& pipe, const PipeEnd end) {
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

void HydraulicConnections::disconnect_all(const NonPipe& element) {
  for (auto& [pipe, end] : element.m_left)
    if (pipe) {
      disconnect(*pipe, end);
    }
  for (auto& [pipe, end] : element.m_right)
    if (pipe) {
      disconnect(*pipe, end);
    }
}
}  // namespace lvtrans
