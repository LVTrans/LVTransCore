#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "lvtrans/elements/pipe.hpp"
namespace lvtrans {

enum class ElementAbstractType {
  Pipe,
  NonPipe,
};

class ElementContainer {
 public:
  ElementContainer() = default;
  ~ElementContainer() = default;
  ElementContainer(ElementContainer&&) = default;
  ElementContainer& operator=(ElementContainer&&) = default;

  template <typename T, typename... Args>
  T* add_element(Args&&... args) {
    return add_element_with_id<T>(m_element_id, std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  T* add_element_with_id(ElementID id, Args&&... args) {
    if (id < 0 || m_element_indices.contains(id)) {
      return nullptr;
    }
    auto element = std::make_unique<T>(std::forward<Args>(args)...);
    auto* ptr = element.get();

    element->set_ID(id);

    if constexpr (std::is_base_of_v<Pipe, T>) {
      m_pipes.push_back(std::move(element));
      m_element_indices[id] = {ElementAbstractType::Pipe, m_pipes.size() - 1};
    } else {
      m_non_pipes.push_back(std::move(element));
      m_element_indices[id] = {ElementAbstractType::NonPipe,
                               m_non_pipes.size() - 1};
    }

    m_element_id = std::max(m_element_id, id + 1);
    return ptr;
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
  template <typename T>
  T* get_element_by_id(ElementID id) const {
    return dynamic_cast<T*>(get_element_by_id(id));
  }

  std::vector<Element*> get_elements() const;
  Element* get_element_by_id(ElementID id) const;
  void remove_element(ElementID id);

 private:
  ElementID m_element_id{0};
  std::vector<std::unique_ptr<Pipe>> m_pipes;
  std::vector<std::unique_ptr<NonPipe>> m_non_pipes;
  std::unordered_map<ElementID, std::pair<ElementAbstractType, size_t>>
      m_element_indices;
};
}  // namespace lvtrans
