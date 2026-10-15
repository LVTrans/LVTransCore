#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_levle_right.hpp"
#include "lvtrans/elements/valve.hpp"
namespace lvtrans {

ElementParseResult ValveParser::parse(ElementContainer& container,
                                      ElementConfig& element, double) {
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
                                     ElementConfig& element, double step_size) {
  auto& config = std::get<PipeParameters>(element.parameters);

  if (!check_parameters(config)) {
    return ElementParseResult::Error;
  }

  InitialPipeState H0 = 0.0;
  InitialPipeState Q0 = 0.0;

  if (element.state) {
    const auto& state = std::get<PipeState>(*element.state);
    if (!check_state(state, config, step_size)) {
      return ElementParseResult::Error;
    }
    config.H0 = state.H;
    config.Q0 = state.Q;
  }

  auto pipe =
      container.add_element_with_id<Pipe>(element.id, config,
                                          step_size);  // TODO: Use system dt
  if (!pipe.has_value()) {
    return ElementParseResult::Error;
  }
  pipe.value()->set_name(element.name);

  return ElementParseResult::Success;
}

ElementParseResult ReservoirParser::parse(ElementContainer& container,
                                          ElementConfig& element, double) {
  const auto& parameters = std::get<ReservoirParameters>(element.parameters);
  Reservoir* reservoir = nullptr;
  if (element.type == ElementType::ConstantLevelRight) {
    reservoir =
        container
            .add_element_with_id<ConstantLevelRight>(element.id, parameters)
            .value_or(nullptr);
  } else {
    reservoir =
        container.add_element_with_id<ConstantLevelLeft>(element.id, parameters)
            .value_or(nullptr);
  }
  if (!reservoir) return ElementParseResult::Error;
  reservoir->set_name(element.name);

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
    default:
      assert(false && "Invalid element type");
      return nullptr;
  }
}
}  // namespace lvtrans
