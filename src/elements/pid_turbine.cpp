#include "lvtrans/elements/pid_turbine.hpp"

namespace lvtrans {

PIDTurbine::PIDTurbine(const PIDTurbineParameters& parameters)
    : m_params{parameters} {}

PIDTurbine::PIDTurbine() = default;
PIDTurbine::~PIDTurbine() = default;

void PIDTurbine::iterate(const double) {}

ElementType PIDTurbine::get_type() const { return ElementType::PIDTurbine; }

ElementParameters PIDTurbine::get_parameters() const { return m_params; }

std::optional<ElementState> PIDTurbine::get_state() const {
  return std::nullopt;
}

ElementView PIDTurbine::read_view() const { return ElementView{}; }

void PIDTurbine::reset_state() {}

}  // namespace lvtrans
