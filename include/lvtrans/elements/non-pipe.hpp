#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
#include "element.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/hydraulic_connections.hpp"

namespace lvtrans {

class BasePipe;

enum class CharacteristicsType { Plus, Minus };

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
  virtual void disconnect_all() noexcept;

  virtual double get_H(CharacteristicsType, ElementID) { return 0.0; }
  virtual double get_Q(CharacteristicsType, ElementID,
                       const IterateInput = {}) {
    return 0.0;
  }

  void set_characteristics(Characteristics c, int id) {
    if (c.type == CharacteristicsType::Plus) {
      m_c_plus.insert_or_assign(id, c);
    } else {
      m_c_minus.insert_or_assign(id, c);
    }
  }

  const BasePipe* left(std::size_t index = 0) const {
    return m_left.empty() ? nullptr : m_left[index].pipe;
  }

  const BasePipe* right(std::size_t index = 0) const {
    return m_right.empty() ? nullptr : m_right[index].pipe;
  }

 protected:
  NonPipe(std::size_t left_count, std::size_t right_count)
      : m_left(left_count), m_right(right_count) {}

  std::unordered_map<ElementID, Characteristics> m_c_plus{};
  std::unordered_map<ElementID, Characteristics> m_c_minus{};

 private:
  std::vector<PipeLink> m_left;
  std::vector<PipeLink> m_right;
  friend class HydraulicConnections;
};
}  // namespace lvtrans
