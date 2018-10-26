#include <iostream>
#include <fstream>
#include <cstdlib>
#include "file_utils.hpp"

bool check_if_file_exists(const std::string& filename) {
  std::ifstream ifile(filename);
  return static_cast<bool>(ifile);
}

bool create_dir(std::string dirname) {
  if (!dirname.empty() && (dirname.back() == '/')) {
    dirname.pop_back();
  }

  if (dirname.empty()) {
    return true;
  }

  const bool file_exists = check_if_file_exists(dirname);

  if (file_exists) {
    if (!check_if_file_exists(dirname)) {
      return false;
    } else {
      return true;
    }
  }

  std::string cmd = std::string("mkdir -p ") + dirname;
  char command_string[cmd.size()+1];
  std::copy(cmd.begin(), cmd.end(), &command_string[0]);
  command_string[cmd.size()] = '\0';
  const int r = ::system(&command_string[0]);

  if (WEXITSTATUS(r) == 0x10) {
    return true;
  } else if (!check_if_file_exists(dirname)) {
    return false;
  }
  return true;
}
