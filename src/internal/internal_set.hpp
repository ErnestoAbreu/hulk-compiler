#ifndef HULK_INTERNAL_SET_HPP
#define HULK_INTERNAL_SET_HPP 1

#include <string>
#include <unordered_set>
#include <vector>

namespace hulk {

namespace internal {

template <class T>
struct set {
  std::unordered_set<T> values;
  bool contains_eps = false;

  set() = default;

  set(std::vector<T> _values, bool _contains_eps = false)
      : contains_eps(_contains_eps) {
    for (auto &value : _values) values.insert(value);
  }

  bool insert(T value) {
    auto result = values.insert(value);
    return result.second;
  }

  bool insert(std::vector<T> _values) {
    bool result = false;
    for (auto &v : _values) result |= insert(v);
    return result;
  }

  bool set_epsilon(bool ncontains_eps) {
    if (ncontains_eps == contains_eps) return false;
    contains_eps = ncontains_eps;
    return true;
  }

  bool merge_values(const set<T> &other) {
    bool result = false;
    for (auto &v : other.values) result |= insert(v);
    return result;
  }

  bool merge_epsilon(const set<T> &other) {
    return set_epsilon(contains_eps | other.contains_eps);
  }

  bool merge(const set<T> &other) {
    return merge_values(other) | merge_epsilon(other);
  }

  bool contains(const T &value) {
    return values.find(value) != end(values);
  }

  T *find(const T &value) {
    for (auto &v : values) {
      if (v == value) return &v;
    }
    return nullptr;
  }

  size_t size() const { return values.size() + static_cast<int>(contains_eps); }

  std::string to_string() const {
    std::string result = "{";
    bool f = true;
    for (auto &v : values) {
      if (!f) result += ", ";
      result += v.to_string();
      f = false;
    }
    result += "} - " + std::to_string(contains_eps);
    return result;
  }

  bool empty() const { return size() == 0; }

  bool operator==(const set<T> &other) {
    return values == other.values && contains_eps == other.contains_eps;
  }

  bool operator==(const std::unordered_set<T> &_values) const {
    return values == _values;
  }

  size_t hash() const { return std::hash<std::string>{}(to_string()); }

  auto begin() { return values.begin(); }
  auto end() { return values.end(); }
};

}  // namespace internal

}  // namespace hulk

#endif  // HULK_INTERNAL_SET_HPP