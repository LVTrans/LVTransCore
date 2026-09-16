#include "lvtrans/plant.hpp"
#include <optional>

namespace lvtrans {

Plant::Plant() : m_state{} {}

Plant::Plant(std::string_view file_path) : Plant() {}

void Plant::add_element(std::shared_ptr<Element> element) {
  if (!element) {
    return;
  }
  element->set_ID(s_element_id);
  m_elements[s_element_id++] = element;
}

void Plant::remove_element(ElementID id) {
  for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
    if (it->second->get_ID() == id) {
      m_elements.erase(it);
      return;
    }
  }
}

std::optional<std::shared_ptr<Element>> Plant::get_element_by_id(ElementID id) {
  auto it = m_elements.find(id);

  if (it != m_elements.end()) {
    return it->second;
  }
  return std::nullopt;
}

} // namespace lvtrans
