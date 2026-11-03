#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/element_parameters.hpp"

namespace lvtrans {

PIDValve::PIDValve(const PIDValveParameters& parameters)
    : m_params{parameters},
      m_derived{PIDValveDerievedData{
          .Tt = parameters.Ti * 0.3,
          .SP_q = parameters.SP_Q / parameters.Q_wanted}} {}

PIDValve::PIDValve() = default;
PIDValve::~PIDValve() = default;

void PIDValve::iterate(const double) {}

ElementType PIDValve::get_type() const { return ElementType::PIDValve; }

ElementParameters PIDValve::get_parameters() const { return m_params; }

std::optional<ElementState> PIDValve::get_state() const { return std::nullopt; }

ElementView PIDValve::read_view() const { return ElementView{}; }

void PIDValve::reset_state() {}

}  // namespace lvtrans
