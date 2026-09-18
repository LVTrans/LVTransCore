#pragma once

namespace lvtrans {

class Element;

class Port {
public:
  Element &owner;
  Port *connected_to{nullptr};

  Port() = delete;
  ~Port() = default;
  explicit Port(Element &comp) : owner(comp) {}
  void connect(Port *other) { connected_to = other; }
};

} // namespace lvtrans
