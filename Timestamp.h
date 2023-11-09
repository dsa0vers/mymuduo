#pragma once

#include <iostream>
#include <string>

namespace mymuduo {
class Timestamp {
 private:
  /* data */
  int64_t microSecondsSinceEpoch_;

 public:
  Timestamp(/* args */);
  explicit Timestamp(int64_t microSecondsSinceEpoch);
  static Timestamp now();
  std::string toString();
};

}  // namespace mymuduo