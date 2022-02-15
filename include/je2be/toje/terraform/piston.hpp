#pragma once

namespace je2be::toje {

class Piston {
  Piston() = delete;

public:
  static void Do(mcfile::je::Chunk &chunkJ, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;
    using namespace props;

    if (!accessor.fHasPiston) {
      return;
    }

    int cx = chunkJ.fChunkX;
    int cz = chunkJ.fChunkZ;
    auto chunkB = cache.at(cx, cz);
    if (!chunkB) {
      return;
    }

    mcfile::je::SetBlockOptions sbo;
    sbo.fRemoveTileEntity = false;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!BlockPropertyAccessor::IsPiston(p)) {
            continue;
          }
          auto blockB = cache.blockAt(x, y, z);
          if (!blockB) {
            continue;
          }
          auto blockJ = chunkJ.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);
          auto blockEntity = chunkB->blockEntityAt(x, y, z);
          if ((blockB->fName == "minecraft:piston" || blockB->fName == "minecraft:sticky_piston") && blockEntity) {
            // Block
            auto state = blockEntity->byte("State");
            props["extended"] = ToString(state == 2);
            auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
            chunkJ.setBlockAt(x, y, z, replace, sbo);

            // Tile entity
            if (state == 3) {
              int facingDirectionB = blockB->fStates->int32("facing_direction", 0);
              bool extending = false;

              auto tileEntityJ = make_shared<CompoundTag>();
              tileEntityJ->set("id", String("minecraft:piston"));
              tileEntityJ->set("keepPacked", Bool(false));
              tileEntityJ->set("x", Int(x));
              tileEntityJ->set("y", Int(y));
              tileEntityJ->set("z", Int(z));
              tileEntityJ->set("extending", Bool(extending));
              tileEntityJ->set("progress", Float(0));
              tileEntityJ->set("facing", Int(facingDirectionB));
              tileEntityJ->set("source", Bool(true));

              map<string, string> blockStateProps;
              blockStateProps["extended"] = ToString(extending);
              blockStateProps["facing"] = JavaNameFromFacing6(Facing6FromBedrockFacingDirectionB(facingDirectionB));
              auto blockState = make_shared<mcfile::je::Block const>(blockB->fName, blockStateProps);

              tileEntityJ->set("blockState", blockState->toCompoundTag());

              chunkJ.fTileEntities[Pos3i(x, y, z)] = tileEntityJ;
            }
          } else if (blockB->fName == "minecraft:movingBlock" && blockEntity) {
            auto piston = PistonBodyFromPistonPos(*blockEntity, cache);
            if (piston) {
              // Block
              props.clear();

              int facingDirectionB = piston->fBlock->fStates->int32("facing_direction", 0);
              Facing6 pistonFacing = Facing6FromBedrockFacingDirectionB(facingDirectionB);
              props["facing"] = JavaNameFromFacing6(pistonFacing);

              bool sticky = piston->fBlock->fName == "minecraft:sticky_piston";
              props["type"] = sticky ? "sticky" : "normal";

              auto replace = make_shared<mcfile::je::Block const>("minecraft:moving_piston", props);
              chunkJ.setBlockAt(x, y, z, replace, sbo);

              // Tile entity
              auto state = piston->fBlockEntity->byte("State");
              bool extending = state == 1; // state should be 3 or 1 here.
              auto tileEntityJ = make_shared<CompoundTag>();
              tileEntityJ->set("id", String("minecraft:piston"));
              tileEntityJ->set("keepPacked", Bool(false));
              tileEntityJ->set("x", Int(x));
              tileEntityJ->set("y", Int(y));
              tileEntityJ->set("z", Int(z));
              tileEntityJ->set("extending", Bool(extending));
              tileEntityJ->set("progress", Float(0));
              tileEntityJ->set("facing", Int(facingDirectionB));
              tileEntityJ->set("source", Bool(false));

              auto movingBlockJ = MovingBlock(*blockEntity);
              if (movingBlockJ) {
                tileEntityJ->set("blockState", movingBlockJ->toCompoundTag());
              }

              chunkJ.fTileEntities[Pos3i(x, y, z)] = tileEntityJ;
            }
          } else if (blockB->fName == "minecraft:pistonArmCollision" || blockB->fName == "minecraft:stickyPistonArmCollision") {
            int facingDirectionB = blockB->fStates->int32("facing_direction", 0);
            Facing6 f6 = Facing6FromBedrockFacingDirectionB(facingDirectionB);
            Pos3i direction = Pos3iFromFacing6(f6);
            Pos3i pistonPos = Pos3i(x, y, z) - direction;
            auto pistonBlockEntity = cache.blockEntityAt(pistonPos);
            if (pistonBlockEntity) {
              auto state = pistonBlockEntity->byte("State");
              if (state == 1) {
                // Block
                // state = 1 means the piston is extending state.
                // Block name shold be renamed to "moving_piston".
                props.clear();
                props["facing"] = JavaNameFromFacing6(f6);
                bool sticky = blockB->fName == "minecraft:stickyPistonArmCollision";
                props["type"] = sticky ? "sticky" : "normal";
                auto replace = make_shared<mcfile::je::Block const>("minecraft:moving_piston", props);
                chunkJ.setBlockAt(x, y, z, replace, sbo);

                // Tile Entity
                auto tileEntityJ = make_shared<CompoundTag>();
                tileEntityJ->set("id", String("minecraft:piston"));
                tileEntityJ->set("keepPacked", Bool(false));
                tileEntityJ->set("x", Int(x));
                tileEntityJ->set("y", Int(y));
                tileEntityJ->set("z", Int(z));
                tileEntityJ->set("progress", Float(0));
                tileEntityJ->set("facing", Int(facingDirectionB));
                tileEntityJ->set("source", Bool(true));
                tileEntityJ->set("extending", Bool(false));
                chunkJ.fTileEntities[Pos3i(x, y, z)] = tileEntityJ;
              }
            }
          }
        }
      }
    }
  }

  static std::shared_ptr<mcfile::je::Block const> MovingBlock(CompoundTag const &blockEntity) {
    auto movingBlock = blockEntity.compoundTag("MovingBlock");
    if (!movingBlock) {
      return nullptr;
    }
    auto movingBlockB = mcfile::be::Block::FromCompound(*movingBlock);
    if (!movingBlockB) {
      return nullptr;
    }
    return BlockData::From(*movingBlockB);
  }

  struct PistonBody {
    std::shared_ptr<mcfile::be::Block const> fBlock;
    std::shared_ptr<CompoundTag const> fBlockEntity;
  };

  static std::optional<PistonBody> PistonBodyFromPistonPos(CompoundTag const &blockEntity, ChunkCache<3, 3> &loader) {
    auto x = blockEntity.int32("pistonPosX");
    auto y = blockEntity.int32("pistonPosY");
    auto z = blockEntity.int32("pistonPosZ");
    if (!x || !y || !z) {
      return std::nullopt;
    }
    auto block = loader.blockAt(*x, *y, *z);
    if (!block) {
      return std::nullopt;
    }
    auto be = loader.blockEntityAt(Pos3i(*x, *y, *z));
    if (!be) {
      return std::nullopt;
    }
    PistonBody pb;
    pb.fBlock = block;
    pb.fBlockEntity = be;
    return pb;
  }

  static std::string ToString(bool b) {
    return b ? "true" : "false";
  }
};

} // namespace je2be::toje