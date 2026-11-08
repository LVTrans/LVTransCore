#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

// LVTrans manual section 12.14: One inlet and two outlets; third branch drawn
// north. Supports connection topology; solver and serialization are not yet
// implemented and report unsupported operations with std::logic_error.
class T1In2OutN : public NonPipe {
 public:
  explicit T1In2OutN(const TConnectionParameters& parameters);
  const TConnectionParameters& config() const { return m_params; }

  void iterate(const double t = 0) override;
  ElementType get_type() const override { return ElementType::T1In2OutN; }
  ElementParameters get_parameters() const override { return m_params; }
  std::optional<ElementState> get_state() const override;
  ElementView read_view() const override;
  void reset_state() override {}
  ModificationResult modify(const ElementModification&) override {
    return ModificationResult::Ok;
  }

 private:
  TConnectionParameters m_params;
};

}  // namespace lvtrans
