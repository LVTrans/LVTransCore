#include "lvtrans/elements/t_1in_2out_n.hpp"
#include <stdexcept>

namespace lvtrans {

T1In2OutN::T1In2OutN(const TConnectionParameters& parameters)
    : NonPipe(1, 2), m_params(parameters) {}

void T1In2OutN::iterate(const double) {
  throw std::logic_error("T1In2OutN solver is not implemented");
}

ElementType T1In2OutN::get_type() const {
  throw std::logic_error("T1In2OutN has no ElementType mapping yet");
}

ElementParameters T1In2OutN::get_parameters() const {
  throw std::logic_error(
      "TConnectionParameters is not in ElementParameters yet");
}

std::optional<ElementState> T1In2OutN::get_state() const {
  return std::nullopt;
}

ElementView T1In2OutN::read_view() const {
  return ElementView{.element_id = get_ID(), .values = {}};
}

}  // namespace lvtrans
