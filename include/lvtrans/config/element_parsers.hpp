#pragma once

#include <cstdint>
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/element_container.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/valve.hpp"
namespace lvtrans {

enum class ElementParseResult : std::uint8_t { Success, Error };

class ElementParser {
 public:
  virtual ~ElementParser() = default;
  virtual ElementParseResult parse(ElementContainer& container,
                                   const ElementConfig& config) = 0;
};

class ValveParser : public ElementParser {
 public:
  ElementParseResult parse(ElementContainer& container,
                           const ElementConfig& element) override;
};

class PipeParser : public ElementParser {
 public:
  ElementParseResult parse(ElementContainer& container,
                           const ElementConfig& element) override;
  bool check_parameters(const PipeParameters& parameters) const {
    if (parameters.num_reaches < 2 || parameters.num_reaches % 2 != 0 ||
        !std::isfinite(parameters.length) || parameters.length <= 0 ||
        !std::isfinite(parameters.diameter) || parameters.diameter <= 0 ||
        !std::isfinite(parameters.a) || parameters.a <= 0) {
      return false;
    }
    return true;
  }
  bool check_state(const PipeState& state, const PipeParameters& config) const {
    if (state.H.size() != config.num_reaches + 1 ||
        state.Q.size() != config.num_reaches + 1) {
      return false;
    }

    return true;
  }
};

class ReservoirParser : public ElementParser {
 public:
  ElementParseResult parse(ElementContainer& container,
                           const ElementConfig& element) override;
};

// create a parser depending on the element type
class ParserFactory {
 public:
  static std::unique_ptr<ElementParser> create(ElementType type);
};

}  // namespace lvtrans
