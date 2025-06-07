#ifndef HULK_INTERNAL_UTIL_HPP
#define HULK_INTERNAL_UTIL_HPP 1

namespace hulk {

namespace internal {

template <class container_t, class range_t>
container_t to(range_t &&range) {
  return container_t(begin(range), end(range));
}

}  // namespace internal

}  // namespace hulk

#endif  // HULK_INTERNAL_UTIL_HPP