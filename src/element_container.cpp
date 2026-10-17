#include "lvtrans/element_container.hpp"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>
namespace lvtrans {

std::vector<Element*> ElementContainer::get_elements_sorted() const {
  auto elements = get_elements();

  std::sort(elements.begin(), elements.end(),
            [](Element* a, Element* b) { return a->get_ID() < b->get_ID(); });
  return elements;
}

std::vector<Element*> ElementContainer::get_elements() const {
  std::vector<Element*> elements{};
  elements.reserve(m_pipes.size() + m_non_pipes.size());

  for (const auto& pipe : m_pipes) {
    elements.emplace_back(pipe.get());
  }

  for (const auto& non_pipe : m_non_pipes) {
    elements.emplace_back(non_pipe.get());
  }

  for (const auto& non_pipe : m_controllers) {
    elements.emplace_back(non_pipe.get());
  }

  return elements;
}

Element* ElementContainer::get_element_by_id(ElementID id) const {
  auto it = m_element_indices.find(id);

  if (it == m_element_indices.end()) {
    return nullptr;
  }

  const auto index = it->second.second;

  if (it->second.first == ElementAbstractType::Pipe) {
    return m_pipes[index].get();
  } else if (it->second.first == ElementAbstractType::NonPipe) {
    return m_non_pipes[index].get();
  } else {
    return m_controllers[index].get();
  }
}

void ElementContainer::remove_element_by_id(ElementID id) {
  auto it = m_element_indices.find(id);
  if (it == m_element_indices.end()) {
    return;
  }

  if (it->second.first == ElementAbstractType::Pipe) {
    remove_element_from(m_pipes, it->second.second);
  } else if (it->second.first == ElementAbstractType::NonPipe) {
    remove_element_from(m_non_pipes, it->second.second);
  } else {
    remove_element_from(m_controllers, it->second.second);
  }
}
}  // namespace lvtrans
