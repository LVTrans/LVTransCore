#include "lvtrans/plant.hpp"
#include "lvtrans/element.hpp"
#include <iostream>

namespace lvtrans {

Plant::Plant(double dt) : m_state{}, m_element_container{} {
  m_state.time_step = dt;
}

Plant::Plant(std::string_view, double dt) : Plant(dt) {}

void Plant::step() {

  m_state.curent_time += m_state.time_step;

  for (auto &pipe : m_element_container.get_pipes()) {
    pipe->iterate();
  }

  for (auto &non_pipe : m_element_container.get_non_pipes()) {
    IterateInput input{};
    input.t = m_state.curent_time;
    non_pipe->iterate(input);
  }
}

void Plant::run_steps(size_t num_steps) {
  for (size_t i = 0; i < num_steps; ++i) {
    step();
  }
}

void Plant::display() {
  for (const auto *element : get_elements()) {
    std::cout << "Element " << element->get_ID();
    std::cout << " (Type: " << typeid(*element).name() << ")\n";
  }

  std::cout << "\n------\n";
  for (auto &pipe : m_element_container.get_pipes()) {
    auto *left_elem = pipe->left_elem();
    if (left_elem) {
      std::cout << "R(" << left_elem->get_ID() << ")<---->";
    }
    std::cout << "P(" << pipe->get_ID() << ")";
    auto *right_elem = pipe->right_elem();
    if (right_elem) {
      std::cout << "<---->V(" << right_elem->get_ID() << ")\n";
    }
  }
}

} // namespace lvtrans
