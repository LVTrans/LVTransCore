#pragma once
#include <span>
#include <unordered_map>
#include <vector>
#include "element.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/hydraulic_connections.hpp"

namespace lvtrans {

class BasePipe;

enum class CharacteristicsType { CPlus, CMinus };

struct Characteristics {
  double c{};
  double b{};
  CharacteristicsType type{};
};

class NonPipe : public Element {
 public:
  ~NonPipe() override;
  std::span<const PipeLink> left_links() const { return m_left; }
  std::span<const PipeLink> right_links() const { return m_right; }
  void disconnect_all() noexcept;

  virtual double get_H(CharacteristicsType, ElementID) const { return 0.0; }
  virtual double get_Q(CharacteristicsType, ElementID,
                       const IterateInput = {}) const {
    return 0.0;
  }
  // void set_c_characteristics(double c) { m_c_characteristics = c; }
  // void set_b_characteristics(double b) { m_b_characteristics = b; }
  void set_characteristics(Characteristics c, int id) {
    if (c.type == CharacteristicsType::CPlus) {
      m_c_plus[id] = c;
    } else {
      m_c_minus[id] = c;
    }
  }

 protected:
  NonPipe(std::size_t left_count, std::size_t right_count)
      : m_left(left_count), m_right(right_count) {}

  std::unordered_map<ElementID, Characteristics> m_c_plus{};
  std::unordered_map<ElementID, Characteristics> m_c_minus{};
  // std::vector<Characteristics> m_c_minus{};

 private:
  std::vector<PipeLink> m_left;
  std::vector<PipeLink> m_right;
  friend class HydraulicConnections;
};
}  // namespace lvtrans
