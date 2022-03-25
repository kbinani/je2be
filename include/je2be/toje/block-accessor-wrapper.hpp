#pragma once

namespace je2be::toje {

template <size_t width, size_t height>
class BlockAccessorWrapper : public je2be::terraform::BlockAccessor {
public:
  explicit BlockAccessorWrapper(je2be::terraform::bedrock::BlockAccessorBedrock<width, height> &base) : fBase(base) {}

  std::shared_ptr<mcfile::je::Block const> blockAt(int x, int y, int z) override {
    auto b = fBase.blockAt(x, y, z);
    if (!b) {
      return nullptr;
    }
    return BlockData::From(*b);
  }

private:
  je2be::terraform::bedrock::BlockAccessorBedrock<width, height> &fBase;
};

} // namespace je2be::toje