#include <je2be/box360/terraform.hpp>

#include <je2be/box360/chunk.hpp>
#include <je2be/box360/progress.hpp>
#include <je2be/future-support.hpp>
#include <je2be/nullable.hpp>
#include <je2be/poi-blocks.hpp>
#include <je2be/terraform/box360/attached-stem.hpp>
#include <je2be/terraform/box360/block-accessor-box360.hpp>
#include <je2be/terraform/box360/chest.hpp>
#include <je2be/terraform/box360/kelp.hpp>
#include <je2be/terraform/chorus-plant.hpp>
#include <je2be/terraform/fence-connectable.hpp>
#include <je2be/terraform/leaves.hpp>
#include <je2be/terraform/note-block.hpp>
#include <je2be/terraform/redstone-wire.hpp>
#include <je2be/terraform/shape-of-stairs.hpp>
#include <je2be/terraform/snowy.hpp>
#include <je2be/terraform/wall-connectable.hpp>

#include <BS_thread_pool.hpp>

namespace je2be::box360 {

class Terraform::Impl {
  Impl() = delete;

  enum {
    cx0 = -32,
    cz0 = -32,
    cx1 = 31,
    cz1 = 31,

    width = cx1 - cx0 + 1,
    height = cz1 - cz0 + 1,
  };

public:
  static Status Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset) {
    PoiBlocks poi;
    Status st = MultiThread(poi, directory, concurrency, progress, progressChunksOffset);
    if (!st.ok()) {
      return st;
    }
    if (poi.write(poiDirectory, Chunk::kTargetDataVersion)) {
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

private:
  static Status MultiThread(PoiBlocks &poi, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset) {
    using namespace std;
    bool running[width][height];
    bool done[width][height];
    for (int i = 0; i < width; i++) {
      fill_n(running[i], height, false);
      fill_n(done[i], height, false);
    }
    unique_ptr<BS::thread_pool> queue(new BS::thread_pool(concurrency));
    deque<future<Nullable<Result>>> futures;
    bool ok = true;
    optional<Status> error;
    int count = 0;
    while (ok) {
      vector<future<Nullable<Result>>> drain;
      FutureSupport::Drain(concurrency + 1, futures, drain);
      for (auto &f : drain) {
        auto result = f.get();
        if (result) {
          result->fPoi.mergeInto(poi);
          MarkFinished(result->fChunk.fX, result->fChunk.fZ, done, running);
        } else {
          if (!error) {
            error = result.status();
          }
          ok = false;
        }
        count++;
      }
      if (!ok || IsComplete(done)) {
        break;
      }
      if (progress && !progress->report(count + progressChunksOffset, 8192 * 3)) {
        ok = false;
        break;
      }
      auto next = NextQueue(done, running);
      if (next) {
        MarkRunning(next->fX, next->fZ, running);
        futures.push_back(queue->submit(DoChunk, next->fX, next->fZ, directory));
      } else {
        assert(!futures.empty());
        auto result = futures.front().get();
        if (result) {
          result->fPoi.mergeInto(poi);
          MarkFinished(result->fChunk.fX, result->fChunk.fZ, done, running);
        } else {
          ok = false;
        }
        count++;
        futures.pop_front();
        if (!ok || IsComplete(done)) {
          break;
        }
        if (progress && !progress->report(count + progressChunksOffset, 8192 * 3)) {
          ok = false;
          break;
        }
      }
    }
    for (auto &f : futures) {
      auto result = f.get();
      if (result) {
        result->fPoi.mergeInto(poi);
      } else {
        ok = false;
        if (!error) {
          error = result.status();
        }
      }
      count++;
      if (ok && progress) {
        if (!progress->report(count + progressChunksOffset, 8192 * 3)) {
          ok = false;
        }
      }
    }
    if (!ok) {
      return error ? *error : JE2BE_ERROR;
    }
    return Status::Ok();
  }

  static void MarkFinished(int cx, int cz, bool done[width][height], bool running[width][height]) {
    int x = cx - cx0;
    int z = cz - cz0;
    done[x][z] = true;

    for (int i = -1; i <= 1; i++) {
      int tx = x + i;
      if (tx < 0 || width <= tx) {
        continue;
      }
      for (int j = -1; j <= 1; j++) {
        int tz = z + j;
        if (tz < 0 || height <= tz) {
          continue;
        }
        running[tx][tz] = false;
      }
    }
  }

  static void MarkRunning(int cx, int cz, bool running[width][height]) {
    int x = cx - cx0;
    int z = cz - cz0;

    for (int i = -1; i <= 1; i++) {
      int tx = x + i;
      if (tx < 0 || width <= tx) {
        continue;
      }
      for (int j = -1; j <= 1; j++) {
        int tz = z + j;
        if (tz < 0 || height <= tz) {
          continue;
        }
        running[tx][tz] = true;
      }
    }
  }

  static bool IsComplete(bool done[width][height]) {
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        if (!done[i][j]) {
          return false;
        }
      }
    }
    return true;
  }

  static std::optional<Pos2i> NextQueue(bool done[width][height], bool running[width][height]) {
    using namespace std;
    for (int x = 0; x < width; x++) {
      for (int z = 0; z < height; z++) {
        if (done[x][z]) {
          continue;
        }
        bool ok = true;
        for (int i = -1; i <= 1 && ok; i++) {
          int tx = x + i;
          if (tx < 0 || width <= tx) {
            continue;
          }
          for (int j = -1; j <= 1; j++) {
            int tz = z + j;
            if (tz < 0 || height <= tz) {
              continue;
            }
            if (running[tx][tz]) {
              ok = false;
              break;
            }
          }
        }
        if (ok) {
          return Pos2i(cx0 + x, cz0 + z);
        }
      }
    }
    return nullopt;
  }

  struct Result {
    Pos2i fChunk;
    PoiBlocks fPoi;
  };

  static Nullable<Result> DoChunk(int cx, int cz, std::filesystem::path const &directory) {
    using namespace std;
    using namespace je2be::terraform;
    using namespace je2be::terraform::box360;

    auto cache = make_shared<terraform::box360::BlockAccessorBox360<3, 3>>(cx - 1, cz - 1, directory);
    auto file = directory / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
    auto input = make_shared<mcfile::stream::FileInputStream>(file);
    auto root = CompoundTag::ReadCompressed(*input, mcfile::Endian::Big);
    Result ret;
    ret.fChunk = Pos2i(cx, cz);
    if (!root) {
      return ret;
    }
    input.reset();
    auto chunk = mcfile::je::WritableChunk::MakeChunk(cx, cz, root);
    if (!chunk) {
      return JE2BE_NULLABLE_NULL;
    }
    cache->set(cx, cz, chunk);

    BlockPropertyAccessorJava accessor(*chunk);

    ShapeOfStairs::Do(*chunk, *cache, accessor);
    FenceConnectable::Do(*chunk, *cache, accessor);
    RedstoneWire::Do(*chunk, *cache, accessor);
    ChorusPlant::Do(*chunk, *cache, accessor);
    WallConnectable::Do(*chunk, *cache, accessor);
    Kelp::Do(*chunk, accessor);
    Snowy::Do(*chunk, *cache, accessor);
    AttachedStem::Do(*chunk, *cache, accessor);
    Leaves::Do(*chunk, *cache, accessor);
    Chest::Do(*chunk, *cache, accessor);
    NoteBlock::Do(*chunk, *cache, accessor);

    for (auto const &section : chunk->fSections) {
      if (!section) {
        continue;
      }
      bool hasPoiBlock = false;
      section->eachBlockPalette([&hasPoiBlock](shared_ptr<mcfile::je::Block const> const &block, size_t) {
        if (PoiBlocks::Interest(*block)) {
          hasPoiBlock = true;
          return false;
        }
        return true;
      });
      if (hasPoiBlock) {
        for (int y = 0; y < 16; y++) {
          for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
              auto block = section->blockAt(x, y, z);
              if (!block) {
                continue;
              }
              if (!PoiBlocks::Interest(*block)) {
                continue;
              }
              int bx = chunk->fChunkX * 16 + x;
              int by = section->y() * 16 + y;
              int bz = chunk->fChunkZ * 16 + z;
              ret.fPoi.add({bx, by, bz}, block->fId);
            }
          }
        }
      }
    }

    auto output = make_shared<mcfile::stream::FileOutputStream>(file);
    if (!chunk->write(*output)) {
      return JE2BE_NULLABLE_NULL;
    }

    return ret;
  }
};

Status Terraform::Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset) {
  return Impl::Do(dim, poiDirectory, directory, concurrency, progress, progressChunksOffset);
}

} // namespace je2be::box360