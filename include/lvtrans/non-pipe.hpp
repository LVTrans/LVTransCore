#pragma once
#include "element.hpp"
#include <memory>

namespace lvtrans {

class NonPipe : public Element {
public:
  virtual double get_H() const = 0;
  virtual double get_Q(const IterateInput input = {}) const = 0;
  void set_left(std::shared_ptr<Element> elem) { m_left_elem = elem; }
  void set_right(std::shared_ptr<Element> elem) { m_right_elem = elem; }
  void set_c_characteristics(double c) { m_c_characteristics = c; }
  void set_b_characteristics(double b) { m_b_characteristics = b; }

protected:
  std::shared_ptr<Element> m_left_elem;
  std::shared_ptr<Element> m_right_elem;
  double m_c_characteristics;
  double m_b_characteristics;
};
} // namespace lvtrans
