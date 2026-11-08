#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_states.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

class T1In2Out : public NonPipe {
 public:
  explicit T1In2Out(const TConnectionParameters& parameters);
  const TConnectionParameters& config() const { return m_params; }
  virtual ~T1In2Out() = default;

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
  double calculate_q();
  TConnectionParameters m_params;
  TConnectiontate m_state{};
};

class T1In2OutN : public T1In2Out {
 public:
  explicit T1In2OutN(const TConnectionParameters& parameters)
      : T1In2Out(parameters) {}
  T1In2OutN() : T1In2Out(TConnectionParameters{}) {}
  ElementType get_type() const override { return ElementType::T1In2OutN; }
};

class T1In2OutS : public T1In2Out {
 public:
  explicit T1In2OutS(const TConnectionParameters& parameters)
      : T1In2Out(parameters) {}
  T1In2OutS() : T1In2Out(TConnectionParameters{}) {}
  ElementType get_type() const override { return ElementType::T1In2OutS; }
};

}  // namespace lvtrans
