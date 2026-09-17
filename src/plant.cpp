#include "lvtrans/plant.hpp"
#include "lvtrans/element.hpp"
#include <iostream>

namespace lvtrans {

Plant::Plant() : m_state{}, m_element_container{} {}

Plant::Plant(std::string_view) : Plant() {}

void Plant::step(double t) {
  for (auto &pipe : m_element_container.get_pipes()) {
    pipe->iterate();
  }

  for (auto &non_pipe : m_element_container.get_non_pipes()) {
    IterateInput input{};
    input.t = t;
    non_pipe->iterate(input);
  }
}

void Plant::run_steps(size_t num_steps) {
  for (size_t i = 0; i < num_steps; ++i) {
    step(0.0);
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
