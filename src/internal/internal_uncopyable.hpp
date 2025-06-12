#ifndef HULK_INTERNAL_UNCOPYABLE_HPP
#define HULK_INTERNAL_UNCOPYABLE_HPP 1

namespace hulk {

namespace internal {

struct uncopyable {
  uncopyable(const uncopyable &) = delete;
  uncopyable &operator=(const uncopyable &) = delete;

 protected:
  uncopyable() = default;
  virtual ~uncopyable() = default;
};

}  // namespace internal

}  // namespace hulk

#endif  // HULK_INTERNAL_UNCOPYABLE_HPP