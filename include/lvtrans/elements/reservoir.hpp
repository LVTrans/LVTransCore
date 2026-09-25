#pragma once
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"
namespace lvtrans {

class Reservoir : public NonPipe {
 public:
  Reservoir(const double elevation);
  Reservoir(const ReservoirParameters& config);
  ~Reservoir();
  void iterate(const IterateInput = {}, IterateOutput = {}) override {};
  ElementType get_type() override { return ElementType::Reservoir; }
  ElementParameters get_parameters() const override { return m_config; }
  std::optional<ElementState> get_state() const override { return {}; }

  double get_H() const override { return m_config.H0; }
  double get_Q(const IterateInput = {}) const override {
    return (m_config.H0 - m_c_characteristics) / m_b_characteristics;
  }

 private:
  ReservoirParameters m_config;
};
}  // namespace lvtrans
