#pragma once

#include <cstdint>
#include <filesystem>
namespace lvtrans {
struct PlantData;
struct ElementConfig;
class ElementContainer;
enum class ElementType : std::uint8_t;

enum class PlantRepositoryResult : std::uint8_t { Ok, Error };

class PlantConfigRepository {
 public:
  PlantConfigRepository() = delete;
  ~PlantConfigRepository() = delete;

  static PlantRepositoryResult load(const std::filesystem::path& path,
                                    PlantData& plant);
  static PlantRepositoryResult save(const std::filesystem::path& path,
                                    const PlantData& plant);
};
}  // namespace lvtrans
