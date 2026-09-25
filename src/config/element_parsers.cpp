#include "lvtrans/config/element_parsers.hpp"
namespace lvtrans {

ElementParseResult ValveParser::parse(ElementContainer& container,
                                      const ElementConfig& element) {
  const auto& config = std::get<ValveParameters>(element.parameters);
  const auto state = element.state ? std::get<ValveState>(*element.state)
                                   : ValveState{config.tau_i};
  auto valve = container.add_element_with_id<Valve>(element.id, config, state);

  if (!valve.has_value()) {
    return ElementParseResult::Error;
  }

  valve.value()->set_name(element.name);

  return ElementParseResult::Success;
}

ElementParseResult PipeParser::parse(ElementContainer& container,
                                     const ElementConfig& element) {
  const auto& config = std::get<PipeParameters>(element.parameters);

  if (!check_parameters(config)) {
    return ElementParseResult::Error;
  }

  InitialPipeParams H0 = 0.0;
  InitialPipeParams Q0 = 0.0;

  if (element.state) {
    const auto& state = std::get<PipeState>(*element.state);
    if (!check_state(state, config)) {
      return ElementParseResult::Error;
    }
    H0 = state.H;
    Q0 = state.Q;
  }

  auto pipe = container.add_element_with_id<Pipe>(element.id, config, H0, Q0);
  if (!pipe.has_value()) {
    return ElementParseResult::Error;
  }
  pipe.value()->set_name(element.name);

  return ElementParseResult::Success;
}

ElementParseResult ReservoirParser::parse(ElementContainer& container,
                                          const ElementConfig& element) {
  auto reservoir = container.add_element_with_id<Reservoir>(
      element.id, std::get<ReservoirParameters>(element.parameters));
  if (!reservoir.has_value()) {
    return ElementParseResult::Error;
  }

  reservoir.value()->set_name(element.name);
  return ElementParseResult::Success;
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
