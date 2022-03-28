#pragma once

namespace je2be::terraform::bedrock {

template <size_t width, size_t height>
class BlockAccessorBedrock {
public:
  BlockAccessorBedrock(mcfile::Dimension d, int cx, int cz, leveldb::DB *db, mcfile::Endian endian) : fDim(d), fCache(width * height), fCacheLoaded(width * height, false), fChunkX(cx), fChunkZ(cz), fDb(db), fEndian(endian) {
  }

  std::shared_ptr<mcfile::be::Chunk> at(int cx, int cz) const {
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    return fCache[*index];
  }

  std::shared_ptr<mcfile::be::Block const> blockAt(int bx, int by, int bz) {
    int cx = mcfile::Coordinate::ChunkFromBlock(bx);
    int cz = mcfile::Coordinate::ChunkFromBlock(bz);
    auto const &chunk = ensureLoadedAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockAt(bx, by, bz);
  }

  std::shared_ptr<CompoundTag const> blockEntityAt(Pos3i const &pos) {
    int cx = mcfile::Coordinate::ChunkFromBlock(pos.fX);
    int cz = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
    auto const &chunk = ensureLoadedAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockEntityAt(pos);
  }

  std::shared_ptr<mcfile::be::Chunk> ensureLoadedAt(int cx, int cz) {
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    if (!fCacheLoaded[*index]) {
      fCache[*index] = mcfile::be::Chunk::Load(cx, cz, fDim, fDb, fEndian);
      fCacheLoaded[*index] = true;
    }
    return fCache[*index];
  }

  std::optional<int> index(int cx, int cz) const {
    int x = cx - fChunkX;
    int z = cz - fChunkZ;
    if (0 <= x && x < width && 0 <= z && z < height) {
      return z * width + x;
    } else {
      return std::nullopt;
    }
  }

  BlockAccessorBedrock<width, height> *makeRelocated(int cx, int cz) const {
    std::unique_ptr<BlockAccessorBedrock<width, height>> ret(new BlockAccessorBedrock<width, height>(fDim, cx, cz, fDb, fEndian));
    for (int x = 0; x < width; x++) {
      for (int z = 0; z < height; z++) {
        auto index = this->index(fChunkX + x, fChunkZ + z);
        if (fCacheLoaded[*index]) {
          auto const &chunk = fCache[*index];
          ret->set(fChunkX + x, fChunkZ + z, chunk);
        }
      }
    }
    return ret.release();
  }

private:
  void set(int cx, int cz, std::shared_ptr<mcfile::be::Chunk> const &chunk) {
    auto index = this->index(cx, cz);
    if (!index) {
      return;
    }
    fCache[*index] = chunk;
    fCacheLoaded[*index] = true;
  }

public:
  mcfile::Dimension const fDim;
  int const fChunkX;
  int const fChunkZ;

private:
  std::vector<std::shared_ptr<mcfile::be::Chunk>> fCache;
  std::vector<bool> fCacheLoaded;
  leveldb::DB *const fDb;
  mcfile::Endian const fEndian;
};

} // namespace je2be::terraform::bedrock
