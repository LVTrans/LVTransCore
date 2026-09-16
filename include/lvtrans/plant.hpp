#pragma once

#include "lvtrans/element.hpp"
#include <optional>
#include <string_view>
#include <unordered_map>
namespace lvtrans {

struct PlantState {
  double time_step{};
  double curent_time{};
  int num_iterations{};
};

class Plant {
public:
  using Elements = std::unordered_map<ElementID, std::shared_ptr<Element>>;

  Plant();
  Plant(std::string_view file_path);

  void add_element(std::shared_ptr<Element> element);
  void remove_element(ElementID id);
  const Elements &get_elements() const { return m_elements; }
  std::optional<std::shared_ptr<Element>> get_element_by_id(ElementID id);

private:
  PlantState m_state;
  Elements m_elements;
  inline static ElementID s_element_id{0};
};
} // namespace lvtrans
