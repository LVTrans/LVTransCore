
#pragma once
namespace lvtrans {
class Controller {
 public:
  virtual void foo() = 0;
  virtual ~Controller() = default;
};
}  // namespace lvtrans
