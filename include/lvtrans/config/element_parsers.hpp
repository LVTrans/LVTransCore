#pragma once

#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/element_container.hpp"
namespace lvtrans {

class ElementParser {
 public:
  virtual ~ElementParser() = default;
  virtual void parse(ElementContainer& container,
                     const ElementConfig& config) = 0;
};

class ValveParser : public ElementParser {
 public:
  void parse(ElementContainer& container,
             const ElementConfig& element) override;
};

class PipeParser : public ElementParser {
 public:
  void parse(ElementContainer& container,
             const ElementConfig& element) override;
};

class ReservoirParser : public ElementParser {
 public:
  void parse(ElementContainer& container,
             const ElementConfig& element) override;
};

// create a parser depending on the element type
class ParserFactory {
 public:
  static std::unique_ptr<ElementParser> create(ElementType type);
};

}  // namespace lvtrans
