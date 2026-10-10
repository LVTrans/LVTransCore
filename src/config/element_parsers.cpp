#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/valve.hpp"
namespace lvtrans {

ElementParseResult ValveParser::parse(ElementContainer& container,
                                      const ElementConfig& element, double) {
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
                                     const ElementConfig& element,
                                     double step_size) {
  const auto& config = std::get<PipeParameters>(element.parameters);

  if (!check_parameters(config)) {
    return ElementParseResult::Error;
  }

  InitialPipeValue H0 = 0.0;
  InitialPipeValue Q0 = 0.0;

  if (element.state) {
    const auto& state = std::get<PipeState>(*element.state);
    if (!check_state(state, config, step_size)) {
      return ElementParseResult::Error;
    }
    H0 = state.H;
    Q0 = state.Q;
  }

  auto pipe =
      container.add_element_with_id<Pipe>(element.id, config, H0, Q0,
                                          step_size);  // TODO: Use system dt
  if (!pipe.has_value()) {
    return ElementParseResult::Error;
  }
  pipe.value()->set_name(element.name);

  return ElementParseResult::Success;
}

ElementParseResult ReservoirParser::parse(ElementContainer& container,
                                          const ElementConfig& element,
                                          double) {
  auto reservoir = container.add_element_with_id<ConstantLevelLeft>(
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
    case ElementType::AlgebraicPipe:
      return std::make_unique<PipeParser>();
    case ElementType::Reservoir:
    case ElementType::ConstantLevelLeft:
    case ElementType::ConstantLevelRight:
      return std::make_unique<ReservoirParser>();
    case ElementType::Valve:
      return std::make_unique<ValveParser>();
  }
  assert(false && "Invalid element type");
  return nullptr;
}
}  // namespace lvtrans
