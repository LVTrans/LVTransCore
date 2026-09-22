#pragma once
#include "lvtrans/element.hpp"
#include "lvtrans/non-pipe.hpp"
namespace lvtrans {

struct ReservoirParameters : LossCoefficients {
  double H0{};  //< Nominal geodesic level of the reservoir from datum.
};

class Reservoir : public NonPipe {
 public:
  Reservoir(const double elevation);
  Reservoir(const ReservoirParameters& config);
  void iterate(const IterateInput = {}, IterateOutput = {}) override {};
  ~Reservoir();
  double get_H() const override { return m_config.H0; }
  double get_Q(const IterateInput = {}) const override {
    return (m_config.H0 - m_c_characteristics) / m_b_characteristics;
  }

 private:
  ReservoirParameters m_config;
};
}  // namespace lvtrans
