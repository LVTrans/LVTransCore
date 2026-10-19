#pragma once
#include <cstddef>

namespace lvtrans {
class BasePipe;
class NonPipe;

enum class Side { Left, Right };
enum class PipeEnd { Left, Right };

struct TerminalId {
  Side side{};
  size_t index{};
  bool operator==(const TerminalId&) const = default;
};

struct BoundaryLink {
  NonPipe* element{nullptr};
  TerminalId terminal{};
};

struct PipeLink {
  BasePipe* pipe{nullptr};
  PipeEnd end{};
};

enum class ConnectResult { Ok, InvalidTerminal, AlreadyConnected };

class HydraulicConnections {
 public:
  static ConnectResult connect(BasePipe& pipe, PipeEnd end, NonPipe& element,
                               size_t terminal_index = 0);
  static void disconnect(BasePipe& pipe, PipeEnd end);
  static void disconnect_all(BasePipe& pipe);
  static void disconnect_all(const NonPipe& element);
};
}  // namespace lvtrans
