#pragma once

namespace je2be::tobe {

class WorldData {
public:
  WorldData(std::filesystem::path const &input, InputOption const &opt) : fInput(input), fJavaEditionMap(input, opt), fInputOption(opt) {}

  void put(DbInterface &db, mcfile::nbt::CompoundTag const &javaLevelData) {
    fPortals.putInto(db);
    fJavaEditionMap.each([this, &db](int32_t mapId) {
      auto found = fMapItems.find(mapId);
      if (found == fMapItems.end()) {
        return;
      }
      Map::Convert(mapId, *found->second, fInput, fInputOption, db);
    });
    putAutonomousEntities(db);

    auto theEnd = LevelData::TheEndData(javaLevelData, fAutonomousEntities.size(), fEndPortalsInEndDimension);
    if (theEnd) {
      db.put(mcfile::be::DbKey::TheEnd(), *theEnd);
    } else {
      db.del(mcfile::be::DbKey::TheEnd());
    }

    fStructures.put(db);

    auto mobEvents = LevelData::MobEvents(javaLevelData);
    if (mobEvents) {
      db.put(mcfile::be::DbKey::MobEvents(), *mobEvents);
    } else {
      db.del(mcfile::be::DbKey::MobEvents());
    }
  }

private:
  void putAutonomousEntities(DbInterface &db) {
    using namespace mcfile::nbt;
    using namespace mcfile::stream;

    auto list = std::make_shared<ListTag>(Tag::Type::Compound);
    for (auto const &e : fAutonomousEntities) {
      list->push_back(e);
    }
    auto root = std::make_shared<CompoundTag>();
    root->set("AutonomousEntityList", list);

    auto s = std::make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    root->writeAsRoot(w);

    std::vector<uint8_t> buffer;
    s->drain(buffer);

    leveldb::Slice v((char const *)buffer.data(), buffer.size());
    db.put(mcfile::be::DbKey::AutonomousEntities(), v);
  }

private:
  std::filesystem::path fInput;

public:
  Portals fPortals;
  JavaEditionMap fJavaEditionMap;
  std::unordered_map<int32_t, std::shared_ptr<mcfile::nbt::CompoundTag>> fMapItems;
  std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  InputOption fInputOption;
  Structures fStructures;
  Statistics fStat;
  int64_t fMaxChunkLastUpdate = 0;
};

} // namespace je2be::tobe