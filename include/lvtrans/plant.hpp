#pragma once
#include <string>
#include "lvtrans/element.hpp"
#include "lvtrans/element_container.hpp"
#include "lvtrans/non-pipe.hpp"
#include "lvtrans/pipe.hpp"
#include "lvtrans/plant_types.hpp"
namespace lvtrans {

struct PlantData {
  ElementContainer element_container;
  PlantState state;
  SimulationConfig config;
};

class Plant {
 public:
  explicit Plant(double step_size);
  explicit Plant(PlantData& data);
  explicit Plant(const std::string& file_path);

  template <typename T, typename... Args>
  T* add_element(Args&&... args) {
    return m_data.element_container.add_element<T>(std::forward<Args>(args)...);
  }
  const std::vector<std::unique_ptr<Pipe>>& get_pipes() const {
    return m_data.element_container.get_pipes();
  }
  const std::vector<std::unique_ptr<NonPipe>>& get_non_pipes() const {
    return m_data.element_container.get_non_pipes();
  }
  std::vector<Element*> get_elements() const {
    return m_data.element_container.get_elements();
  }
  Element* get_element_by_id(ElementID id) {
    return m_data.element_container.get_element_by_id(id);
  }

  template <typename T>
  T* get_element_by_id(ElementID id) {
    return m_data.element_container.get_element_by_id<T>(id);
  }

  void remove_element(ElementID id) {
    return m_data.element_container.remove_element(id);
  }

  void step();
  void run_steps(size_t num_steps);
  void display();
  double get_current_time() const { return m_data.state.current_time; }

 private:
  PlantData m_data;
};
}  // namespace lvtrans
