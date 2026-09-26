#pragma once
#include <string>
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/element_container.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/plant_types.hpp"
namespace lvtrans {

struct PlantData {
  ElementContainer element_container;
  PlantState state;
  SimulationConfig config;
  PlantMetaData meta;
  int format_version{1};
};

class Plant {
 public:
  Plant() = default;
  ~Plant() = default;
  explicit Plant(double step_size);
  explicit Plant(const std::string& file_path);
  // Note: must be called with rvalue reference to avoid copy (std::move)
  explicit Plant(PlantData&& data);

  template <typename T, typename... Args>
  std::optional<T*> add_element(Args&&... args) {
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
  std::optional<Element*> get_element_by_id(ElementID id) {
    return m_data.element_container.get_element_by_id(id);
  }

  template <typename T>
  std::optional<T*> get_element_by_id(ElementID id) {
    return m_data.element_container.get_element_by_id<T>(id);
  }

  void remove_element(ElementID id) {
    return m_data.element_container.remove_element(id);
  }

  PlantRepositoryResult save(const std::string& file_path);
  void step();
  void run_steps(size_t num_steps);
  void display() const;
  double get_current_time() const { return m_data.state.current_time; }

 private:
  PlantData m_data;
};
}  // namespace lvtrans
