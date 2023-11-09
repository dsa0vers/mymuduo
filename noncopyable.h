#pragma once

/**
 * noncopyable实现继承的所有派生类只能进行析构和析构，
 * 而无法进行拷贝构造和赋值操作的功能
 */
namespace mymuduo {
class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};
}  // namespace mymuduo
