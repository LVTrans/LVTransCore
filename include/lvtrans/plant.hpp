#pragma once
#include "lvtrans/element.hpp"
#include "lvtrans/element_container.hpp"
#include "lvtrans/non-pipe.hpp"
#include "lvtrans/pipe.hpp"
#include <string_view>
namespace lvtrans {

struct PlantState {
  double time_step{};
  double curent_time{};
  int num_iterations{};
};

class Plant {
public:
  Plant();
  Plant(std::string_view file_path);

  template <typename T, typename... Args> T &add_element(Args &&...args) {
    return m_element_container.add_element<T>(std::forward<Args>(args)...);
  }
  const std::vector<std::unique_ptr<Pipe>> &get_pipes() const {
    return m_element_container.get_pipes();
  }
  const std::vector<std::unique_ptr<NonPipe>> &get_non_pipes() const {
    return m_element_container.get_non_pipes();
  }
  std::vector<Element *> get_elements() const {
    return m_element_container.get_elements();
  }
  Element *get_element_by_id(ElementID id) {
    return m_element_container.get_element_by_id(id);
  }
  void remove_element(ElementID id) {
    return m_element_container.remove_element(id);
  }

  void step(double t);
  void run_steps(size_t num_steps);
  void display();

private:
  PlantState m_state;
  ElementContainer m_element_container;
};
} // namespace lvtrans
