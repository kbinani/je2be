#pragma once

namespace je2be::via {

class BackwardConverter {
  BackwardConverter() = delete;

public:
  enum class Version : uint8_t {
    Version1_19 = 1,
    Version1_18 = 2,
    Version1_17 = 3,
    Version1_16_2 = 4,
    Version1_16 = 5,
    Version1_15 = 6,
    Version1_14 = 7,
    Version1_13_2 = 8,
    Version1_13 = 9,
    Version1_12 = 10,
    Version1_11 = 11,
    Version1_10 = 12,
    Version1_9_4 = 13,
  };
};

} // namespace je2be::via
