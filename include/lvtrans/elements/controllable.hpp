#pragma once

namespace lvtrans {
class Controllable {
 public:
  virtual void bar() = 0;
  virtual ~Controllable() = default;
};
}  // namespace lvtrans
