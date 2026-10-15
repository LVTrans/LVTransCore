#pragma once
#include <optional>
#include <string>
#include "lvtrans/element_modifications.hpp"
#include "lvtrans/element_types.hpp"

namespace lvtrans {

struct IterateOutput {
  double H{};
  double Q{};
};

struct IterateInput {
  double t{};
  double H{};
  double Q{};
  double B{};
  double R{};
};

class Element {
 public:
  Element() = default;
  Element(const Element&) = delete;
  Element& operator=(const Element&) = delete;
  Element(Element&&) = delete;
  Element& operator=(Element&&) = delete;
  virtual ~Element() = default;
  virtual void iterate(const double = 0) {};
  virtual ElementType get_type() const = 0;
  virtual ElementParameters get_parameters() const = 0;
  virtual std::optional<ElementState> get_state() const { return std::nullopt; }
  virtual ElementView read_view() const { return ElementView{}; }
  virtual void reset_state() {};
  virtual ModificationResult modify(const ElementModification&) {
    return ModificationResult::Ok;
  }

  void set_name(const std::string& name) { m_name = name; }
  std::string get_name() const { return m_name; }
  ElementID get_ID() const { return m_ID; }
  void set_ID(ElementID id) { m_ID = id; }

 protected:
  ElementID m_ID{};
  std::string m_name{};
};
}  // namespace lvtrans
