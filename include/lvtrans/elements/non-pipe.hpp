#pragma once
#include "element.hpp"
#include "lvtrans/port.hpp"

namespace lvtrans {

class NonPipe : public Element {
 public:
  virtual double get_H() const = 0;
  virtual double get_Q(const IterateInput input = {}) const = 0;
  void set_c_characteristics(double c) { m_c_characteristics = c; }
  void set_b_characteristics(double b) { m_b_characteristics = b; }

 protected:
  double m_c_characteristics;
  double m_b_characteristics;
};
}  // namespace lvtrans
