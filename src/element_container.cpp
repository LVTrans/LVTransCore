#include "lvtrans/element_container.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
namespace lvtrans {

std::vector<Element *> ElementContainer::get_elements() const {
  std::vector<Element *> elements{};
  elements.reserve(m_pipes.size() + m_non_pipes.size());

  for (const auto &pipe : m_pipes) {
    elements.push_back(pipe.get());
  }

  for (const auto &non_pipe : m_non_pipes) {
    elements.push_back(non_pipe.get());
  }

  return elements;
}

Element *ElementContainer::get_element_by_id(ElementID id) {
  auto it = m_element_indices.find(id);

  if (it == m_element_indices.end()) {
    return nullptr;
  }

  if (it->second.first == ElementBaseType::Pipe) {
    return m_pipes[it->second.second].get();
  } else {
    return m_non_pipes[it->second.second].get();
  }
}

void ElementContainer::remove_element(ElementID id) {
  auto it = m_element_indices.find(id);
  if (it == m_element_indices.end()) {
    return;
  }

  if (it->second.first == ElementBaseType::Pipe) {
    remove_element_from(m_pipes, it->second.second);
  } else {
    remove_element_from(m_non_pipes, it->second.second);
  }
}
} // namespace lvtrans
