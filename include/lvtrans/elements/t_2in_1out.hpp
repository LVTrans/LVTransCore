#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_states.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

class T2In1Out : public NonPipe {
 public:
  explicit T2In1Out(const TConnectionParameters& parameters);
  const TConnectionParameters& config() const { return m_params; }
  virtual ~T2In1Out() = default;

  void iterate(const double t = 0) override;
  ElementParameters get_parameters() const override { return m_params; }
  std::optional<ElementState> get_state() const override;
  ElementView read_view() const override;
  double get_H(CharacteristicsType ct, ElementID id) override;
  double get_Q(CharacteristicsType ct, ElementID id,
               IterateInput input) override;
  void reset_state() override { m_state = TConnectiontate{}; }

  ModificationResult modify(const ElementModification&) override {
    return ModificationResult::Ok;
  }

 private:
  void calculate_q();
  TConnectionParameters m_params;
  TConnectiontate m_state{};
};

class T2In1OutN : public T2In1Out {
 public:
  explicit T2In1OutN(const TConnectionParameters& parameters)
      : T2In1Out(parameters) {}
  T2In1OutN() : T2In1Out(TConnectionParameters{}) {}
  ElementType get_type() const override { return ElementType::T2In1OutN; }
};

class T2In1OutS : public T2In1Out {
 public:
  explicit T2In1OutS(const TConnectionParameters& parameters)
      : T2In1Out(parameters) {}
  T2In1OutS() : T2In1Out(TConnectionParameters{}) {}
  ElementType get_type() const override { return ElementType::T2In1OutS; }
};

}  // namespace lvtrans
