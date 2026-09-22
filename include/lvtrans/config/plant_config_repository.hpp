#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
namespace lvtrans {
struct PlantData;
struct ElementConfig;
class ElementContainer;
enum class ElementType : std::uint8_t;

enum class PlantRepositoryResult : std::uint8_t { Ok, Error };

class PlantConfigRepository {
 public:
  PlantConfigRepository();
  ~PlantConfigRepository() = default;

  [[nodiscard]] PlantRepositoryResult load(const std::filesystem::path& path,
                                           PlantData& plant);
  [[nodiscard]] PlantRepositoryResult save(const std::filesystem::path& path,
                                           const PlantData& plant);
};
}  // namespace lvtrans
