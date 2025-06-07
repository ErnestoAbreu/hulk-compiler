#ifndef HULK_INTERNAL_OVERLOADED_HPP
#define HULK_INTERNAL_OVERLOADED_HPP 1

namespace hulk {

namespace internal {

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace internal

}  // namespace hulk

#endif  // HULK_INTERNAL_OVERLOADED_HPP