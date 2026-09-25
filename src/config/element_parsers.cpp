#include "lvtrans/config/element_parsers.hpp"
namespace lvtrans {

void ValveParser::parse(ElementContainer& container,
                        const ElementConfig& element) {
  const auto& config = std::get<ValveParameters>(element.parameters);
  const auto state = element.state ? std::get<ValveState>(*element.state)
                                   : ValveState{config.tau_i};
  auto* valve = container.add_element_with_id<Valve>(element.id, config, state);
  valve->set_name(element.name);
}

void PipeParser::parse(ElementContainer& container,
                       const ElementConfig& element) {
  const auto& config = std::get<PipeParameters>(element.parameters);

  assert(check_parameters(config) && "Invalid pipe geometry or reach count");

  if (element.state) {
    const auto& state = std::get<PipeState>(*element.state);
    assert(check_state(state, config) && "Invalid pipe state");
    auto* pipe = container.add_element_with_id<Pipe>(element.id, config,
                                                     state.H, state.Q);
    pipe->set_name(element.name);
  } else {
    auto* pipe =
        container.add_element_with_id<Pipe>(element.id, config, 0.0, 0.0);
    pipe->set_name(element.name);
  }
}

void ReservoirParser::parse(ElementContainer& container,
                            const ElementConfig& element) {
  auto* reservoir = container.add_element_with_id<Reservoir>(
      element.id, std::get<ReservoirParameters>(element.parameters));
  reservoir->set_name(element.name);
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
  assert(false && "Invalid element type");
  return nullptr;
}
}  // namespace lvtrans
