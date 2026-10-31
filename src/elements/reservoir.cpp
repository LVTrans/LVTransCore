#include "lvtrans/elements/reservoir.hpp"
#include <format>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

Reservoir::Reservoir(double elevation, std::size_t left_count,
                     std::size_t right_count)
    : NonPipe(left_count, right_count) {
  m_params.H0 = elevation;
}

Reservoir::Reservoir(const ReservoirParameters& params, std::size_t left_count,
                     std::size_t right_count)
    : Reservoir(params.H0, left_count, right_count) {
  m_params = params;
}

Reservoir::~Reservoir() {}

ModificationResult Reservoir::modify(const ElementModification& mod) {
  if (std::get_if<SetReservoirH0>(&mod)) {
    m_params.H0 = std::get<SetReservoirH0>(mod).h0;
    return ModificationResult::Ok;
  }
  return ModificationResult::UnsupportedModification;
}

ElementView Reservoir::read_view() const {
  return ElementView{
      .element_id = m_ID,
      .values =
          {
              ScalarValue{.name = "Flow Rate",
                          .unit = "m³/s",
                          .symbol = "Q",
                          .value = m_state.Q},
          },
  };
}

}  // namespace lvtrans
