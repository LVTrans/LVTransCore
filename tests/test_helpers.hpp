#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

inline std::string get_mock_data_file_path(std::string_view path) {
  return std::string(TEST_DATA_DIR) + std::string(path);
}

// Returns true if both file contents are line-for-line equal.
inline bool compare_csv_files(const std::string& path1,
                              const std::string& path2) {
  std::ifstream f1(path1);
  std::ifstream f2(path2);

  if (f1.fail()) {
    std::cout << "compare_csv_files failed: " << path1 << " not found\n";
    return false;
  }
  if (f2.fail()) {
    std::cout << "compare_csv_files failed: " << path2 << " not found\n";
    return false;
  }

  int line{1};
  for (std::string line1, line2;; ++line) {
    const bool has_line1 = static_cast<bool>(std::getline(f1, line1));
    const bool has_line2 = static_cast<bool>(std::getline(f2, line2));
    if (f1.bad() || f2.bad()) return false;
    if (has_line1 != has_line2) {
      std::cout << "compare_csv_files failed: line count mismatch\n";
      return false;
    }
    if (!has_line1) return true;
    if (line1 != line2) {
      std::cout << "failed at line " << line << ": " << line1 << " vs " << line2
                << '\n';
      return false;
    }
  }
}
