#include "lvtrans/elements/surge_shaft_standard.hpp"
#include <stdexcept>
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

SurgeShaftStandard::SurgeShaftStandard(
    const SurgeShaftStandardParameters& parameters)
    : NonPipe(1, 1), m_params(parameters) {}

void SurgeShaftStandard::iterate(const double) {
  throw std::logic_error("SurgeShaftStandard solver is not implemented");
}

std::optional<ElementState> SurgeShaftStandard::get_state() const {
  return std::nullopt;
}

ElementView SurgeShaftStandard::read_view() const {
  return ElementView{.element_id = get_ID(), .values = {}};
}

}  // namespace lvtrans
