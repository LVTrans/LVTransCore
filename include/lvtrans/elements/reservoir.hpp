#pragma once
#include "lvtrans/element_modifications.hpp"
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/non-pipe.hpp"
namespace lvtrans {

class Reservoir : public NonPipe {
 public:
  Reservoir(double elevation, std::size_t left_count, std::size_t right_count);
  Reservoir(const ReservoirParameters& config, std::size_t left_count,
            std::size_t right_count);
  ~Reservoir() override = 0;
  void iterate(const double) override {};
  ModificationResult modify(const ElementModification& mod) override;

  ElementType get_type() const override { return ElementType::Reservoir; }
  ElementParameters get_parameters() const override { return m_params; }
  std::optional<ElementState> get_state() const override { return {m_state}; }
  void reset_state() override {}
  ElementView read_view() const override;

 protected:
  ReservoirParameters m_params;
  ReservoirState m_state;
};
}  // namespace lvtrans
