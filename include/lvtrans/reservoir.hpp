#pragma once
#include "lvtrans/element.hpp"
#include "lvtrans/non-pipe.hpp"
#include <cstdlib>
namespace lvtrans {

class Reservoir : public NonPipe {
public:
  Reservoir(const double elevation);
  ~Reservoir();
  double get_H() const override { return m_elevation; }
  double get_Q(const IterateInput) const override {
    return (m_elevation - m_c_characteristics) / m_b_characteristics;
  }

private:
  double m_elevation{};
};
} // namespace lvtrans
