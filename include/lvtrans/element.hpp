#pragma once
namespace lvtrans {
struct IterateOutput {
  double H{};
  double Q{};
};
struct IterateInput {
  double H{};
  double Q{};
  double B{};
  double R{};
  double t{};
};

class Element {
public:
  virtual void iterate(const IterateInput input = {},
                       IterateOutput output = {}) {};
  virtual ~Element() {};
};
} // namespace lvtrans
