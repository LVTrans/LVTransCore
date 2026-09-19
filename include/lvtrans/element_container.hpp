#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "lvtrans/pipe.hpp"
namespace lvtrans {

enum class ElementBaseType {
  Pipe,
  NonPipe,
};
class ElementContainer {
 public:
  ElementContainer() = default;
  ~ElementContainer() = default;

  template <typename T, typename... Args>
  T& add_element(Args&&... args) {
    auto element = std::make_unique<T>(std::forward<Args>(args)...);

    element->set_ID(m_element_id);
    T& ref = *element;

    if constexpr (std::is_base_of_v<Pipe, T>) {
      m_pipes.push_back(std::move(element));
      m_element_indices[ref.get_ID()] = {ElementBaseType::Pipe,
                                         m_pipes.size() - 1};
    } else {
      m_non_pipes.push_back(std::move(element));
      m_element_indices[ref.get_ID()] = {ElementBaseType::NonPipe,
                                         m_non_pipes.size() - 1};
    }

    m_element_id++;
    return ref;
  }

  template <typename T>
  void remove_element_from(std::vector<std::unique_ptr<T>>& elements,
                           size_t index) {
    if (index < elements.size()) {
      m_element_indices.erase(elements[index]->get_ID());
      elements[index]->reset_ports();

      if (index != elements.size() - 1) {
        std::swap(elements[index], elements.back());
        m_element_indices.at(elements[index]->get_ID()).second = index;
      }

      elements.pop_back();
    }
  }

  const std::vector<std::unique_ptr<Pipe>>& get_pipes() const {
    return m_pipes;
  }
  const std::vector<std::unique_ptr<NonPipe>>& get_non_pipes() const {
    return m_non_pipes;
  }

  std::vector<Element*> get_elements() const;
  Element* get_element_by_id(ElementID id);
  void remove_element(ElementID id);

 private:
  ElementID m_element_id{0};
  std::vector<std::unique_ptr<Pipe>> m_pipes;
  std::vector<std::unique_ptr<NonPipe>> m_non_pipes;
  std::unordered_map<ElementID, std::pair<ElementBaseType, size_t>>
      m_element_indices;
};
}  // namespace lvtrans
