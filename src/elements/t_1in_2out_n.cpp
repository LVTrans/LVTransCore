#include "lvtrans/elements/t_1in_2out_n.hpp"
#include <stdexcept>

namespace lvtrans {

T1In2OutN::T1In2OutN(const TConnectionParameters& parameters)
    : NonPipe(1, 2), m_params(parameters) {}

void T1In2OutN::iterate(const double) {
  throw std::logic_error("T1In2OutN solver is not implemented");
}

std::optional<ElementState> T1In2OutN::get_state() const {
  return std::nullopt;
}

ElementView T1In2OutN::read_view() const {
  return ElementView{.element_id = get_ID(), .values = {}};
}

}  // namespace lvtrans
