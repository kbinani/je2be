#pragma once

namespace j2b {

class HeightMap {
public:
  HeightMap() { std::fill_n(fHeight, 256, 0); }

  void update(int localX, int y, int localZ) {
    assert(0 <= localX && localX < 16);
    assert(0 <= localZ && localZ < 16);
    assert(0 <= y && y < 256);
    size_t i = localZ * 16 + localX;
    int16_t clamped = Clamp(y, (int)std::numeric_limits<int16_t>::lowest(), (int)std::numeric_limits<int16_t>::max());
    int16_t height = (std::max)(fHeight[i], clamped);
    fHeight[i] = height;
  }

  void write(mcfile::stream::OutputStreamWriter &w) {
    size_t i = 0;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++, i++) {
        int16_t h = fHeight[i];
        w.write(h);
      }
    }
  }

private:
  int16_t fHeight[256];
};

} // namespace j2b
