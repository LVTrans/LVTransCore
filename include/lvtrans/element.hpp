#pragma once
namespace lvtrans {
struct IterateOutput {
  double H{};
  double Q{};
};
struct IterateInput {
  double t{};
  double H{};
  double Q{};
  double B{};
  double R{};
};

class Element {
public:
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) {};
  virtual ~Element() {};
};
} // namespace lvtrans
