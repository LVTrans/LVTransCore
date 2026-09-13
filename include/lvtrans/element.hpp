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
  using IDType = int;

public:
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) {};
  virtual ~Element() {};
  IDType get_ID() const { return m_ID; }

protected:
  IDType m_ID;
};
} // namespace lvtrans
