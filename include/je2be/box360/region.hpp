#pragma once

namespace je2be::box360 {

class Region {
  Region() = delete;

public:
  static bool Convert(mcfile::Dimension dimension,
                      std::filesystem::path const &mcr,
                      int rx,
                      int rz,
                      std::filesystem::path const &regionTempDir,
                      Options const &options) {
    using namespace std;
    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++) {
        int cx = rx * 32 + x;
        int cz = rz * 32 + z;
        if (!options.fChunkFilter.empty()) [[unlikely]] {
          if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
            continue;
          }
        }
        shared_ptr<mcfile::je::WritableChunk> chunk;
        if (!Chunk::Convert(dimension, mcr, cx, cz, chunk)) {
          continue;
        }
        if (!chunk) {
          continue;
        }

        auto nbtz = regionTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
        auto stream = make_shared<mcfile::stream::FileOutputStream>(nbtz);
        if (!chunk->write(*stream)) {
          stream.reset();
          Fs::Delete(nbtz);
          return false;
        }
      }
    }
    return true;
  }
};

} // namespace je2be::box360
