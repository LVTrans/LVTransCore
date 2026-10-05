#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

Reservoir::Reservoir(const double elevation) {
  m_params.H0 = elevation;
  m_ports[PortType::Right].emplace(*this, PortType::Right);
}

Reservoir::Reservoir(const ReservoirParameters& params) : Reservoir(params.H0) {
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
              ScalarValue{.name = "Reservoir Head",
                          .unit = "m",
                          .symbol = "HR",
                          .value = m_params.H0},
          },
  };
}

}  // namespace lvtrans
