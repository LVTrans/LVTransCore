#pragma once

#include <string_view>
namespace lvtrans {
class Plant {

public:
  Plant() = default;
  Plant(std::string_view file_path);
};
} // namespace lvtrans
