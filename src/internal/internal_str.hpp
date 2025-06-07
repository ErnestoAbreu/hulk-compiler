#ifndef HULK_INTERNAL_STR_HPP
#define HULK_INTERNAL_STR_HPP 1

#include <bitset>
#include <string>
#include <vector>
#include <algorithm>

namespace hulk {

namespace internal {

std::vector<std::string> split(std::string s, std::string sep = " ") {
  std::vector<std::string> result;
  std::string current;
  for (size_t i = 0; i < size(s);) {
    if (i + size(sep) - 1 < size(s) && s.substr(i, size(sep)) == sep) {
      if (!current.empty()) {
        result.push_back(current);
        current.clear();
        i += size(sep);
      }
    } else {
      current.push_back(s[i]);
      i += 1;
    }
  }
  if (!current.empty()) result.push_back(current);
  return result;
}

std::string strip(std::string s, char c = ' ') {
  for (int step = 0; step < 2; step++) {
    while (!s.empty() && s.back() == c) s.pop_back();
    std::reverse(begin(s), end(s));
  }
  return s;
}

}  // namespace internal

}  // namespace hulk

#endif  // HULK_INTERNAL_STR_HPP