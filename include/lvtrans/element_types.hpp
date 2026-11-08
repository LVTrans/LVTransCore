#pragma once
#include <cstdint>
#include <string_view>

namespace lvtrans {

enum class ElementType : std::uint8_t {
  Pipe,
  AlgebraicPipe,
  Reservoir,
  ValveInternalServo,
  SurgeShaftStandard,
  PID,
  PIDValve,
  PIDTurbine,
  ConstantLevelLeft,
  ConstantLevelRight,
  SimpleConnection,
  T1In2OutN,
  T1In2OutS,
  T2In1OutN,
  T2In1OutS,
};

constexpr std::string_view element_type_str(ElementType type) {
  switch (type) {
    case ElementType::Pipe:
      return "Pipe";
    case ElementType::AlgebraicPipe:
      return "AlgebraicPipe";
    case ElementType::Reservoir:
      return "Reservoir";
    case ElementType::ValveInternalServo:
      return "ValveInternalServo";
    case ElementType::SurgeShaftStandard:
      return "SurgeShaftStandard";
    case ElementType::PID:
      return "PID";
    case ElementType::PIDValve:
      return "PIDValve";
    case ElementType::PIDTurbine:
      return "PIDTurbine";
    case ElementType::ConstantLevelLeft:
      return "ConstantLevelLeft";
    case ElementType::ConstantLevelRight:
      return "ConstantLevelRight";
    case ElementType::SimpleConnection:
      return "SimpleConnection";
    case ElementType::T1In2OutN:
      return "T1In2OutN";
    case ElementType::T1In2OutS:
      return "T1In2OutS";
    case ElementType::T2In1OutN:
      return "T2In1OutN";
    case ElementType::T2In1OutS:
      return "T2In1OutS";
    default:
      return "UnknownElementType";
  }
}

}  // namespace lvtrans
