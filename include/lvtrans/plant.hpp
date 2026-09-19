#pragma once
#include <string_view>
#include "lvtrans/element.hpp"
#include "lvtrans/element_container.hpp"
#include "lvtrans/non-pipe.hpp"
#include "lvtrans/pipe.hpp"
namespace lvtrans {

struct PlantState {
  double time_step{0.1};
  double current_time{0};
  int num_iterations{0};
};

class Plant {
 public:
  Plant(double dt);
  Plant(std::string_view file_path, double dt);

  template <typename T, typename... Args>
  T& add_element(Args&&... args) {
    return m_element_container.add_element<T>(std::forward<Args>(args)...);
  }
  const std::vector<std::unique_ptr<Pipe>>& get_pipes() const {
    return m_element_container.get_pipes();
  }
  const std::vector<std::unique_ptr<NonPipe>>& get_non_pipes() const {
    return m_element_container.get_non_pipes();
  }
  std::vector<Element*> get_elements() const {
    return m_element_container.get_elements();
  }
  Element* get_element_by_id(ElementID id) {
    return m_element_container.get_element_by_id(id);
  }
  void remove_element(ElementID id) {
    return m_element_container.remove_element(id);
  }

  void step();
  void run_steps(size_t num_steps);
  void display();
  double get_current_time() const { return m_state.current_time; }

 private:
  PlantState m_state;
  ElementContainer m_element_container;
};
}  // namespace lvtrans
