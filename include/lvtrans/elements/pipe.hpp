#pragma once
#include <variant>
#include <vector>
#include "lvtrans/const.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/non-pipe.hpp"
namespace lvtrans {

inline constexpr double calculate_R(double f, double dx, double diameter,
                                    double area) {
  return f * dx / (2.0 * consts::g * diameter * area * area);
}
inline constexpr double calculate_dx(double pipe_length, size_t num_reaches) {
  return pipe_length / static_cast<double>(num_reaches);
}

inline constexpr double calculate_pipe_area(double diameter) {
  return consts::pi * diameter * diameter / 4.0;
}

inline constexpr double calculate_B(double a, double area) {
  return a / (consts::g * area);
}

using InitialPipeValue = std::variant<double, std::vector<double>>;

class Pipe : public Element {
 public:
  Pipe(PipeParameters params, InitialPipeValue H0, InitialPipeValue Q0);
  ~Pipe();
  void iterate(const double t = 0) override;
  ElementType get_type() override { return ElementType::Pipe; }
  ElementParameters get_parameters() const override { return m_params; }
  ElementView read_view() const override;
  std::optional<ElementState> get_state() const override {
    return std::make_optional(m_state);
  }
  void reset_state() override { m_state = m_initial_state; }

  const PipeParameters& config() const { return m_params; }
  double get_R() const { return m_R; }
  double get_B() const { return m_B; }
  const std::vector<double>& get_H() const { return m_state.H; }
  const std::vector<double>& get_Q() const { return m_state.Q; }
  double get_latest_H() const { return m_state.H.back(); }
  double get_latest_Q() const { return m_state.Q.back(); }

  void remove_left();
  void remove_right();
  NonPipe* left_elem() const;
  NonPipe* right_elem() const;

 private:
  void initialize_h_q(InitialPipeValue& H0, InitialPipeValue& Q0);

  PipeParameters m_params{};

  const double m_area{};
  const double m_dx{};
  const double m_R{};
  const double m_B{};
  const size_t m_num_nodes{};

  std::vector<double> m_Z{};
  PipeState m_state{};
  PipeState m_initial_state{};
};
}  // namespace lvtrans
