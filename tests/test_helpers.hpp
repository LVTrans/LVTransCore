#pragma once
#include <fstream>
#include <iostream>
#include <string>

inline std::string get_mock_data_file_path(std::string_view path) {
  return std::string(TEST_DATA_DIR) + std::string(path);
}

// Returns true if both file contents are line-for-line equal.
inline bool compare_csv_files(std::string path1, std::string path2) {
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

  for (std::string line1, line2;
       std::getline(f1, line1) && std::getline(f2, line2);) {
    std::cout << line1 << " vs " << line2 << '\n';
    if (line1 != line2) {
      return false;
    }
  }
  return true;
}
