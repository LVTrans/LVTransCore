#pragma once

#include <cstdint>
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/element_container.hpp"
#include "lvtrans/elements/base_pipe.hpp"
namespace lvtrans {

enum class ElementParseResult : std::uint8_t { Success, Error };

class ElementParser {
 public:
  virtual ~ElementParser() = default;
  virtual ElementParseResult parse(ElementContainer& container,
                                   const ElementConfig& config,
                                   double step_size) = 0;
};

class ValveParser : public ElementParser {
 public:
  ElementParseResult parse(ElementContainer& container,
                           const ElementConfig& element,
                           double step_size) override;
};

class PipeParser : public ElementParser {
 public:
  ElementParseResult parse(ElementContainer& container,
                           const ElementConfig& element,
                           double step_size) override;
  bool check_parameters(const PipeParameters& parameters) const {
    if (!std::isfinite(parameters.length) || parameters.length <= 0 ||
        !std::isfinite(parameters.diameter) || parameters.diameter <= 0 ||
        !std::isfinite(parameters.a) || parameters.a <= 0) {
      return false;
    }
    return true;
  }
  bool check_state(const PipeState& state, const PipeParameters&,
                   double) const {
    if (state.H.size() != state.Q.size()) {
      return false;
    }

    for (size_t i = 0; i < state.H.size(); ++i) {
      if (!std::isfinite(state.H[i]) || !std::isfinite(state.Q[i])) {
        return false;
      }
    }

    // TODO: Find a way to enforce this check
    // const auto segments_temp =
    //     calculate_nodes_temp(p.lambda, p.length, step_size, p.a, p.rho);
    // const size_t segments =
    //     static_cast<size_t>(calculate_num_segments(segments_temp));
    // if (segments != state.H.size()) {
    //   return false;
    // }

    return true;
  }
};

class ReservoirParser : public ElementParser {
 public:
  ElementParseResult parse(ElementContainer& container,
                           const ElementConfig& element,
                           double step_size) override;
};

// create a parser depending on the element type
class ParserFactory {
 public:
  static std::unique_ptr<ElementParser> create(ElementType type);
};

}  // namespace lvtrans
