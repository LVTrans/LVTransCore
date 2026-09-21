#pragma once

#include <cstdint>
#include <filesystem>
#include "lvtrans/plant.hpp"
namespace lvtrans {

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
