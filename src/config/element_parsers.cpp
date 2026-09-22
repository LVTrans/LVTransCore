#include "lvtrans/config/element_parsers.hpp"
namespace lvtrans {
void ValveParser::parse(ElementContainer& container,
                        const ElementConfig& element) {
  const auto& config = std::get<ValveParameters>(element.parameters);
  const auto state = element.state ? std::get<ValveState>(*element.state)
                                   : ValveState{config.tau_i};
  container.add_element_with_id<Valve>(element.id, config, state);
}

void PipeParser::parse(ElementContainer& container,
                       const ElementConfig& element) {
  const auto& config = std::get<PipeParameters>(element.parameters);
  if (config.num_reaches < 2 || config.num_reaches % 2 != 0 ||
      !std::isfinite(config.length) || config.length <= 0 ||
      !std::isfinite(config.diameter) || config.diameter <= 0 ||
      !std::isfinite(config.a) || config.a <= 0) {
    throw std::invalid_argument("Invalid pipe geometry or reach count");
  }
  if (element.state) {
    const auto& state = std::get<PipeState>(*element.state);
    if (state.H.size() != config.num_reaches + 1 ||
        state.Q.size() != config.num_reaches + 1) {
      throw std::invalid_argument(
          "Pipe state requires num_reaches + 1 H and Q values");
    }
    // Pipe takes separate head and flow initial values, not a PipeState.
    container.add_element_with_id<Pipe>(element.id, config, state.H, state.Q);
  } else {
    container.add_element_with_id<Pipe>(element.id, config, 0.0, 0.0);
  }
}

void ReservoirParser::parse(ElementContainer& container,
                            const ElementConfig& element) {
  container.add_element_with_id<Reservoir>(
      element.id, std::get<ReservoirParameters>(element.parameters));
}

std::unique_ptr<ElementParser> ParserFactory::create(ElementType type) {
  switch (type) {
    case ElementType::Pipe:
      return std::make_unique<PipeParser>();
    case ElementType::Reservoir:
      return std::make_unique<ReservoirParser>();
    case ElementType::Valve:
      return std::make_unique<ValveParser>();
  }
  throw std::invalid_argument("Unknown element type");
}
}  // namespace lvtrans
