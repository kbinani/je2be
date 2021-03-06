#pragma once

namespace j2b {

class OrientedPortalBlocks {
public:
  explicit OrientedPortalBlocks(bool xAxis) : fXAxis(xAxis) {}

  void add(int x, int y, int z) { fBlocks.emplace(x, y, z); }

  void extract(std::vector<Portal> &buffer, Dimension dim) {
    while (!fBlocks.empty()) {
      Pos start = *fBlocks.begin();
      Pos bottomNorthWest = lookupBottomNorthWestCorner(start);
      Pos topSouthEast = lookupTopSouthEastCorner(start);
      for (int x = bottomNorthWest.fX; x <= topSouthEast.fX; x++) {
        for (int z = bottomNorthWest.fZ; z <= topSouthEast.fZ; z++) {
          for (int y = bottomNorthWest.fY; y <= topSouthEast.fY; y++) {
            Pos p(x, y, z);
            fBlocks.erase(p);
          }
        }
      }
      uint8_t span = (uint8_t)(std::max(topSouthEast.fX - bottomNorthWest.fX, topSouthEast.fZ - bottomNorthWest.fZ) + 1);
      Portal portal((int32_t)dim, span, bottomNorthWest.fX, bottomNorthWest.fY, bottomNorthWest.fZ, fXAxis ? 1 : 0, fXAxis ? 0 : 1);
      buffer.push_back(portal);
    }
  }

  void drain(OrientedPortalBlocks &out) {
    assert(out.fXAxis == fXAxis);
    for (auto b : fBlocks) {
      out.fBlocks.insert(b);
    }
    std::unordered_set<Pos, PosHasher>().swap(fBlocks);
  }

private:
  Pos lookupBottomNorthWestCorner(Pos start) {
    if (fXAxis) {
      return lookupCorner<-1, -1, 0>(start);
    } else {
      return lookupCorner<0, -1, -1>(start);
    }
  }

  Pos lookupTopSouthEastCorner(Pos start) {
    if (fXAxis) {
      return lookupCorner<1, 1, 0>(start);
    } else {
      return lookupCorner<0, 1, 1>(start);
    }
  }

  template <int dx, int dy, int dz>
  Pos lookupCorner(Pos p) {
    int const x0 = p.fX;
    int const y0 = p.fY;
    int const z0 = p.fZ;
    int currentX = x0;
    int currentY = y0;
    int currentZ = z0;
    int width = 1;
    int height = 1;
    while (true) {
      bool expandVertical = true;
      for (int xz = 0; xz < width; xz++) {
        Pos test(x0 + dx * xz, currentY + dy, z0 + dz * xz);
        if (fBlocks.find(test) == fBlocks.end()) {
          expandVertical = false;
          break;
        }
      }
      if (expandVertical) {
        currentY += dy;
        height++;
      }

      bool expandHorizontal = true;
      for (int y = 0; y < height; y++) {
        Pos test(currentX + dx, y0 + dy * y, currentZ + dz);
        if (fBlocks.find(test) == fBlocks.end()) {
          expandHorizontal = false;
          break;
        }
      }
      if (!expandVertical && !expandHorizontal) {
        break;
      }

      if (expandHorizontal) {
        width++;
        currentX += dx;
        currentZ += dz;
      }
    }
    return Pos(currentX, currentY, currentZ);
  }

private:
  bool const fXAxis;
  std::unordered_set<Pos, PosHasher> fBlocks;
};

} // namespace j2b
