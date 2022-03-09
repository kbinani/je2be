#pragma once

static optional<wstring> GetLocalApplicationDirectory() {
#if __has_include(<shlobj_core.h>)
  int csidType = CSIDL_LOCAL_APPDATA;
  wchar_t path[MAX_PATH + 256];

  if (SHGetSpecialFolderPathW(nullptr, path, csidType, FALSE)) {
    return wstring(path);
  }
#endif
  return nullopt;
}

static optional<fs::path> GetWorldDirectory(string const &name) {
  auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
  if (!appDir) {
    return nullopt;
  }
  return fs::path(*appDir) / L"Packages" / L"Microsoft.MinecraftUWP_8wekyb3d8bbwe" / L"LocalState" / L"games" / L"com.mojang" / L"minecraftWorlds" / name / L"db";
}

static leveldb::DB *OpenF(fs::path p) {
  using namespace leveldb;
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, p, &db);
  if (!st.ok()) {
    return nullptr;
  }
  return db;
}

static leveldb::DB *Open(string const &name) {
  using namespace leveldb;
  auto dir = GetWorldDirectory(name);
  if (!dir) {
    return nullptr;
  }
  return OpenF(*dir);
}

static void VisitDbUntil(string const &name, function<bool(string const &key, string const &value, leveldb::DB *db)> callback) {
  using namespace leveldb;
  unique_ptr<DB> db(Open(name));
  if (!db) {
    return;
  }
  ReadOptions ro;
  unique_ptr<Iterator> itr(db->NewIterator(ro));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    string k = itr->key().ToString();
    string v = itr->value().ToString();
    if (!callback(k, v, db.get())) {
      break;
    }
  }
}

static void VisitDb(string const &name, function<void(string const &key, string const &value, leveldb::DB *db)> callback) {
  VisitDbUntil(name, [callback](string const &k, string const &v, leveldb::DB *db) {
    callback(k, v, db);
    return true;
  });
}

static void FenceGlassPaneIronBarsConnectable() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    string name = mcfile::blocks::Name(id);
    if (name.ends_with("_stairs")) {
      continue;
    }
    if (name.ends_with("piston")) {
      continue;
    }
    if (name.ends_with("door")) {
      continue;
    }
    uniq.insert(name);
  }
  vector<string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "place_blocks.mcfunction").string());
    for (string const &name : names) {
      os << "setblock " << x << " " << y << " " << z0 << " " << name << endl;
      x++;
    }
    x1 = x;
    os << "fill " << x0 << " " << y << " " << (z0 + 1) << " " << x1 << " " << y << " " << (z0 + 1) << " glass_pane" << endl;
    os << "fill " << x0 << " " << y << " " << (z0 - 1) << " " << x1 << " " << y << " " << (z0 - 1) << " oak_fence" << endl;
  }

  // login the game, then execute /function je2be:place_blocks

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  set<string> glassPaneAttachable;
  set<string> fenceAttachable;
  for (int x = x0; x < x1; x++) {
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(x, y, z0);
    auto expected = names[x - x0];
    if (expected != center->fName) {
      cerr << "block does not exist: expected=" << expected << "; actual=" << center->fName << endl;
    } else {
      auto fence = chunk->blockAt(x, y, z0 - 1);
      auto fenceAttached = fence->property("south") == "true";
      if (fenceAttached) {
        fenceAttachable.insert(expected);
      }

      auto glassPane = chunk->blockAt(x, y, z0 + 1);
      auto glassPaneAttached = glassPane->property("north") == "true";
      if (glassPaneAttached) {
        glassPaneAttachable.insert(expected);
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsFenceAlwaysAttachable(mcfile::blocks::BlockId id) {" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : fenceAttachable) {
    code << "    case mcfile::blocks::minecraft::" << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      break;" << endl;
  code << "  }" << endl;
  code << "  //TODO:" << endl;
  code << "  return false;" << endl;
  code << "}" << endl;
  code << endl;
  code << "static bool IsGlassPaneOrIronBarsAlwaysAttachable(mcfile::blocks::BlockId id) {" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : glassPaneAttachable) {
    code << "    case mcfile::blocks::minecraft::" << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      break;" << endl;
  code << "  }" << endl;
  code << "  //TODO:" << endl;
  code << "  return false;" << endl;
  code << "}" << endl;
}

static void NoteBlock() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    string name = mcfile::blocks::Name(id);
    uniq.insert(name);
  }
  vector<string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "research_note_block.mcfunction").string());
    for (string const &name : names) {
      os << "setblock " << x << " " << (y - 1) << " " << z0 << " " << name << endl;
      os << "setblock " << x << " " << y << " " << z0 << " note_block" << endl;
      x += 2;
    }
    x1 = x;
  }

  // login the game, then execute /function je2be:research_note_block

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  map<string, set<string>> instruments;
  int i = 0;
  for (int x = x0; x < x1; x += 2, i++) {
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(x, y - 1, z0);
    auto expected = names[i];
    if (expected != center->fName) {
      cerr << "block does not exist: expected=" << expected << "; actual=" << center->fName << endl;
    } else {
      auto noteBlock = chunk->blockAt(x, y, z0);
      auto instrument = noteBlock->property("instrument", "");
      if (instrument.empty()) {
        cerr << "empty instrument: [" << x << ", " << z0 << "]" << endl;
      } else {
        instruments[instrument].insert(center->fName);
      }
    }
  }

  instruments.erase("harp");

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static std::string NoteBlockInstrument(mcfile::blocks::BlockId id) {" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : instruments) {
    string instrument = it.first;
    set<string> const &blocks = it.second;
    for (string const &block : blocks) {
      code << "  case mcfile::blocks::minecraft::" << block.substr(10) << ":" << endl;
    }
    code << "    return \"" + instrument << "\";" << endl;
  }
  code << "  default:" << endl;
  code << "    return \"harp\"" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void RedstoneWire() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    string name = mcfile::blocks::Name(id);
    if (name.ends_with("slab")) {
      continue;
    }
    if (name.ends_with("rail")) {
      continue;
    }
    if (!mcfile::blocks::IsTransparent(id)) {
      continue;
    }
    uniq.insert(name);
  }
  vector<string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "research_redstone_wire.mcfunction").string());
    os << "fill " << x0 << " " << y << " " << (z0 - 1) << " " << (x0 + 2 * names.size()) << " " << (y + 2) << " " << (z0 + 1) << " air" << endl;
    for (string const &name : names) {
      os << "setblock " << x << " " << (y - 1) << " " << z0 << " air" << endl;
      os << "setblock " << x << " " << (y - 2) << " " << z0 << " redstone_wire" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << (z0 - 1) << " redstone_wire" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << (z0 + 1) << " redstone_wire" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << z0 << " " << name << endl;
      x += 2;
    }
    x1 = x0 + 2 * names.size();
  }

  // login the game, then execute /function je2be:research_redstone_wire

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  set<string> transparent;
  int i = 0;
  for (int x = x0; x < x1; x += 2, i++) {
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(x, y - 1, z0);
    auto expected = names[i];
    if (expected != center->fName) {
      cerr << "block does not exist: expected=" << expected << "; actual=" << center->fName << endl;
    } else {
      auto wire = chunk->blockAt(x, y - 2, z0);
      auto north = wire->property("north", "");
      auto south = wire->property("south", "");
      if (north != "up" && south != "up") {
        transparent.insert(expected);
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsAlwaysTransparentAgainstRedstoneWire(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : transparent) {
    code << "  case " << it.substr(10) << ":" << endl;
  }
  code << "    true false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void Data3D() {
  unique_ptr<leveldb::DB> db(Open("1.18be"));
  auto key = mcfile::be::DbKey::Data3D(0, 0, mcfile::Dimension::End);
  leveldb::ReadOptions ro;
  std::string value;
  if (auto st = db->Get(ro, key, &value); !st.ok()) {
    return;
  }
  auto bmap = mcfile::be::BiomeMap::Decode(0, value, 512);
  std::cout << bmap->numSections() << std::endl; // 11(or more) for overworld, 8 for nether, 4 for end
}

static void Structures() {
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;
  unique_ptr<DB> db(Open("1.18stronghold"));
  unique_ptr<Iterator> itr(db->NewIterator({}));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key().ToString();
    auto parsed = DbKey::Parse(key);
    if (!parsed) {
      continue;
    }
    if (!parsed->fIsTagged) {
      continue;
    }
    uint8_t tag = parsed->fTagged.fTag;
    if (tag != static_cast<uint8_t>(DbKey::Tag::StructureBounds)) {
      continue;
    }
    vector<StructurePiece> buffer;
    StructurePiece::Parse(itr->value().ToString(), buffer);
    for (StructurePiece const &p : buffer) {
      switch (p.fType) {
      case StructureType::Fortress:
      case StructureType::Monument:
      case StructureType::Outpost:
        break;
      default:
        auto chunk = parsed->fTagged.fChunk;
        cout << "unknown structure type: " << (int)p.fType << " at chunk (" << chunk.fX << ", " << chunk.fZ << "), block (" << (chunk.fX * 16) << ", " << (chunk.fZ * 16) << ")" << endl;
        break;
      }
    }
  }
}

static void Keys(leveldb::DB &db, unordered_set<string> &buffer) {
  unique_ptr<leveldb::Iterator> itr(db.NewIterator({}));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    buffer.insert(itr->key().ToString());
  }
}

static void Stronghold() {
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;

  fs::path pathB("C:/Users/kbinani/Documents/Projects/je2be-gui/1.18stronghold.after");
  fs::path pathA("C:/Users/kbinani/Documents/Projects/je2be-gui/1.18stronghold.before");
  unique_ptr<DB> b(OpenF(pathB / "db"));
  unique_ptr<DB> a(OpenF(pathA / "db"));

  unordered_set<string> keysA;
  unordered_set<string> keysB;
  Keys(*a, keysA);
  Keys(*b, keysB);

  unordered_set<string> common;
  for (string s : keysA) {
    if (keysB.find(s) == keysB.end()) {
      auto p = DbKey::Parse(s);
      cout << "new: " << p->toString() << endl;
    } else {
      common.insert(s);
    }
  }

  auto levelAStream = make_shared<mcfile::stream::GzFileInputStream>(pathA / "level.dat");
  levelAStream->seek(8);
  auto levelA = CompoundTag::Read(levelAStream, std::endian::little);

  auto levelBStream = make_shared<mcfile::stream::GzFileInputStream>(pathB / "level.dat");
  levelBStream->seek(8);
  auto levelB = CompoundTag::Read(levelBStream, std::endian::little);

  DiffCompoundTag(*levelB, *levelA);
}

static void MonumentBedrock() {
  using namespace std;
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;
  using namespace je2be;
  using namespace je2be::tobe;

  unique_ptr<DB> db(Open("1.18be"));
  unique_ptr<Iterator> itr(db->NewIterator({}));
  vector<Volume> volumes;
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key();
    auto parsed = DbKey::Parse(key.ToString());
    if (!parsed->fIsTagged) {
      continue;
    }
    if (parsed->fTagged.fTag != static_cast<uint8_t>(DbKey::Tag::StructureBounds)) {
      continue;
    }
    string value;
    if (auto st = db->Get({}, key, &value); !st.ok()) {
      continue;
    }
    vector<StructurePiece> buffer;
    StructurePiece::Parse(itr->value().ToString(), buffer);
    for (auto const &it : buffer) {
      if (it.fType != StructureType::Monument) {
        continue;
      }
      Volume v = it.fVolume;
      volumes.push_back(v);
    }
  }
  Volume::Connect(volumes);
  unordered_map<Pos3i, string, Pos3iHasher> blocks;
  unordered_set<string> ignore = {
      "water",
      "kelp",
      "seagrass",
      "dirt",
      "flowing_water",
      "sand",
      "gravel",
      "stone",
      "copper_ore",
      "coal_ore",
      "iron_ore",
      "lapis_ore",
      "sandstone",
      "packed_ice",
      "sponge",
      "ice",
      "blue_ice",
      "gold_block",
  };
  unordered_set<string> target = {
      "prismarine",
      "seaLantern",
  };
  for (Volume const &v : volumes) {
    cout << "=====" << endl;
    Pos2i start(v.fStart.fX, v.fStart.fZ);
    Pos2i end(v.fEnd.fX, v.fEnd.fZ);
    cout << "[" << start.fX << ", " << start.fZ << "] - [" << end.fX << ", " << end.fZ << "]" << endl;

    int x0 = start.fX;
    int z0 = start.fZ;
    int y0 = v.fStart.fY;
    int cx = Coordinate::ChunkFromBlock(x0);
    int cz = Coordinate::ChunkFromBlock(z0);
    toje::ChunkCache<5, 5> cache(Dimension::Overworld, cx, cz, db.get(), std::endian::little);

    string facing;
    if (start.fX == 8891 && start.fZ == 13979) {
      facing = "north"; // OK
    } else if (start.fX == 9435 && start.fZ == 8315) {
      facing = "east"; // OK
    } else if (start.fX == 9275 && start.fZ == 7259) {
      facing = "south"; // OK
    } else if (start.fX == 9195 && start.fZ == 6827) {
      facing = "west"; // OK
    } else if (start == Pos2i(2715, -3349)) {
      facing = "south"; // OK
    } else if (start == Pos2i(763, -629)) {
      facing = "west"; // OK
    } else if (start == Pos2i(747, -341)) {
      // broken
      continue;
    } else if (start == Pos2i(3227, 3563)) {
      facing = "south"; // OK
    } else if (start == Pos2i(9339, 5419)) {
      facing = "east"; // OK
    } else if (start == Pos2i(9771, 5819)) {
      facing = "south"; // OK
    } else if (start == Pos2i(10491, 6651)) {
      facing = "east"; // OK
    } else if (start == Pos2i(8891, 13979)) {
      facing = "north"; // OK
    } else if (start == Pos2i(9275, 7259)) {
      facing = "south"; // OK
    } else if (start == Pos2i(9387, 7819)) {
      facing = "north"; // OK
    } else if (start == Pos2i(9435, 8315)) {
      facing = "east"; // OK
    } else if (start == Pos2i(8747, 9003)) {
      facing = "north"; // OK
    } else if (start == Pos2i(8939, 9307)) {
      facing = "north"; // OK
    } else if (start == Pos2i(9195, 6827)) {
      facing = "west"; // OK
    } else if (start == Pos2i(8491, 8411)) {
      facing = "north"; // OK
    } else if (start == Pos2i(8363, 8859)) {
      facing = "east"; //
    } else {
      cout << "unknown facing monument at (" << start.fX << ", " << start.fZ << ")" << endl;
    }
    if (facing.empty()) {
      continue;
    }
    auto &b = blocks;
    for (int y = v.fStart.fY; y <= v.fEnd.fY; y++) {
      for (int z = z0; z <= end.fZ; z++) {
        for (int x = x0; x <= end.fX; x++) {
          auto bl = cache.blockAt(x, y, z);
          if (!bl) {
            cerr << "cannot get block at [" << x << ", " << y << ", " << z << "]" << endl;
            return;
          }
          string name = bl->fName.substr(10);
          Pos3i local(x - x0, y - y0, z - z0);
          Pos2i radius(29, 29);
          Pos2i xz(local.fX, local.fZ);
          xz = xz - radius;
          int l90 = 0;
          int dx = 0;
          int dz = 0;
          if (facing == "east") {
            l90 = 1;
            dz = -1;
          } else if (facing == "south") {
            l90 = 2;
            dx = -1;
            dz = -1;
          } else if (facing == "west") {
            l90 = 3;
            dx = -1;
          }
          for (int k = 0; k < l90; k++) {
            xz = Left90(xz);
          }
          xz = xz + radius;
          local = Pos3i(xz.fX + dx, local.fY, xz.fZ + dz);
          CHECK(0 <= local.fX);
          CHECK(local.fX < 58);
          CHECK(0 <= local.fY);
          CHECK(local.fY < 23);
          CHECK(0 <= local.fZ);
          CHECK(local.fZ < 58);

          auto found = b.find(local);
          if (ignore.find(name) != ignore.end()) {
            if (found == b.end()) {
              b[local] = "";
            } else if (found->second != "") {
              b[local] = "";
            }
            continue;
          }
          if (target.find(name) != target.end()) {
            if (found == b.end()) {
              b[local] = name;
            } else if (found->second != name) {
              b[local] = "";
            }
            continue;
          }
          cout << "unknown: " << name << " at (" << x << ", " << y << ", " << z << ")" << endl;
          return;
        }
      }
    }
  }
#if 1
  for (auto const &it : blocks) {
    Pos3i p = it.first;
    string block = it.second;
    if (block.empty()) {
      continue;
    }
    cout << "blocks[{" << p.fX << ", " << p.fY << ", " << p.fZ << "}] = \"" << block << "\";" << endl;
  }
#endif
}

static bool ExtractRecursive(je2be::box360::StfsPackage &pkg, je2be::box360::StfsFileListing &listing, std::filesystem::path dir) {
  for (auto &file : listing.fileEntries) {
    auto p = dir / file.name;
    try {
      pkg.ExtractFile(&file, p.string());
    } catch (...) {
      return false;
    }
  }
  for (auto &folder : listing.folderEntries) {
    auto sub = dir / folder.folder.name;
    if (!ExtractRecursive(pkg, folder, sub)) {
      return false;
    }
  }
  return true;
}

static void Box360Chunk() {
  using namespace je2be::box360;
  fs::path dir("C:/Users/kbinani/Documents/Projects/je2be-gui/00000001");
  int cx = 25;
  int cz = 25;
  for (auto name : {
#if 0
          "1-Save20220305225836-000-pig-name=Yahoo.bin",
           "1-Save20220305225836-001-pig-name=Yohoo.bin",
           "1-Save20220305225836-002-put-cobblestone.bin",
           "1-Save20220305225836-003-remove-cobblestone.bin",
           "2-Save20220306005722-000-fill-bedrock-under-sea-level.bin",
           "2-Save20220306005722-001-chunk-filled-by-bedrock.bin",
           //"abc-Save20220303092528.bin",
           "2-Save20220306005722-002-c.25.25-section0-filled-with-bedrock.bin",
           "2-Save20220306005722-003-heightmap-check.bin",
           "2-Save20220306005722-004-place-dirt-lx=15-ly=15-lz=15.bin",
           "2-Save20220306005722-005-place-dirt-lx=14-ly=15-lz=15.bin",
           "2-Save20220306005722-006-place-dirt-lx=13-ly=15-lz=15.bin",
           "2-Save20220306005722-007-place-dirt-lx=0-ly=15-lz=15.bin",
           "2-Save20220306005722-008-fill-dirt-lz=15-ly=15.bin",
           "2-Save20220306005722-009-reset-lz15_ly=15-fill-dirt-l0_ly15.bin",
           "2-Save20220306005722-010-refill-bedrock-again.bin",
           "2-Save20220306005722-011-preparing-empty-chunk-at-c.25.25.bin",
           "2-Save20220306005722-012-empty-chunk-at-c.25.25.bin",
           "2-Save20220306005722-013-just-resaved-after-012.bin",
           "2-Save20220306005722-014-just-resaved-after-013.bin",
           "2-Save20220306005722-015-setblock 0 1 1 bedrock.bin",
           "2-Save20220306005722-016-setblock 1 1 0 bedrock.bin",
           "2-Save20220306005722-017-setblock 2 1 0 bedrock.bin",
           "2-Save20220306005722-018-setblock 0 1 1 bedrock.bin",
           "2-Save20220306005722-019-setblock 0 2 0 bedrock.bin",
           "2-Save20220306005722-020-setblock 0 3 0 bedrock.bin",
           "2-Save20220306005722-021-fill 0 0 0 3 3 3 bedrock.bin",
           "2-Save20220306005722-022-setblock 0 4 0 bedrock.bin",
           "2-Save20220306005722-023-fill 0 1 0 3 4 3 air.bin",
           "2-Save20220306005722-024-setblock 0 1 0 iron_block.bin",
           "2-Save20220306005722-025-setblock 0 1 0 carved_pumpkin[facing=south].bin",
           "2-Save20220306005722-026-setblock 0 1 0 carved_pumpkin[facing=east].bin",
           "2-Save20220306005722-027-setblock 0 1 4 carved_pumpkin[facing=east].bin",
           "2-Save20220306005722-028-setblock 4 1 0 carved_pumpkin[facing=south].bin",
           "2-Save20220306005722-029-setblock 0 1 8 carved_pumpkin[facing=south].bin",
           "2-Save20220306005722-030-setblock 0 1 12 iron_block.bin",
#endif
         "2-Save20220306005722-031-setblock 4 1 0 gold_block.bin",
             "2-Save20220306005722-032-setblock 4 1 4 dirt.bin",
             "2-Save20220306005722-033-put bedrocks to grid corners under sea level.bin",
             "2-Save20220306005722-034-resaved.bin",
             "2-Save20220306005722-035-fill grid(1,0,0) with iron_block.bin",
             "2-Save20220306005722-036-fill grid(1,1,0) with gold_block.bin",
             "2-Save20220306005722-037-fill grid(0,0,0) with bedrock.bin",
       }) {
    cout << name << endl;
    auto temp = File::CreateTempDir(fs::temp_directory_path());
    CHECK(temp);
    defer {
      Fs::Delete(*temp);
    };
    auto savegame = *temp / "savegame.dat";
    CHECK(Savegame::ExtractSavagameFromSaveBin(dir / name, savegame));
    vector<uint8_t> buffer;
    CHECK(Savegame::DecompressSavegame(savegame, buffer));
    CHECK(Savegame::ExtractFilesFromDecompressedSavegame(buffer, *temp));
    auto r00 = *temp / "region" / "r.0.0.mcr";
    CHECK(fs::exists(r00));
    {
      auto f = make_shared<mcfile::stream::FileInputStream>(r00);
      CHECK(Savegame::ExtractRawChunkFromRegionFile(*f, cx, cz, buffer));
    }
    CHECK(buffer.size() > 0);
    auto foo = Savegame::DecompressRawChunk(buffer);
    CHECK(foo > 0);

    string basename = fs::path(name).replace_extension().string();
    //CHECK(make_shared<mcfile::stream::FileOutputStream>(dir / (basename + "-c." + to_string(cx) + "." + to_string(cz) + ".raw.bin"))->write(buffer.data(), buffer.size()));
    Savegame::DecodeDecompressedChunk(buffer);

    string data;
    data.assign((char const *)buffer.data(), buffer.size());
    vector<uint8_t>().swap(buffer);
    auto found = data.find(string("\x0a\x00\x00", 3));
    CHECK(found != string::npos);
    auto sub = data.substr(found);
    auto bs = make_shared<mcfile::stream::ByteStream>(sub);
    auto tag = CompoundTag::Read(bs, endian::big);
    CHECK(tag);
    CHECK(bs->pos() == sub.size());

    buffer.assign(data.begin(), data.begin() + found);
    CHECK(make_shared<mcfile::stream::FileOutputStream>(dir / (basename + "-c." + to_string(cx) + "." + to_string(cz) + ".prefix.bin"))->write(buffer.data(), buffer.size()));
    for (int i = 0; i + 4096 < buffer.size(); i++) {
      uint8_t start = buffer[i];
      bool ok = true;
      for (int j = 1; j < 4096; j++) {
        if (buffer[i + j] != start) {
          ok = false;
          break;
        }
      }
      if (!ok) {
        continue;
      }
      int count = 4096;
      int j = i + 1;
      for (; j + 4096 < buffer.size(); j++) {
        if (buffer[j] == start) {
          count++;
        } else {
          break;
        }
      }
      cout << "0x" << hex << (int)start << dec << " continues " << count << " bytes at " << dec << i << "(0x" << hex << i << dec << ")" << endl;
      i = j;
    }

    uint8_t maybeEndTagMarker = buffer[0];       // 0x00. The presence of this tag prevents the file from being parsed as nbt.
    uint8_t maybeLongArrayTagMarker = buffer[1]; // 0x0c. Legacy parsers that cannot interpret the LongArrayTag will fail here.
    int32_t xPos = mcfile::I32FromBE(*(int32_t *)(buffer.data() + 0x2));
    int32_t zPos = mcfile::I32FromBE(*(int32_t *)(buffer.data() + 0x6));
    int64_t maybeLastUpdate = mcfile::I64FromBE(*(int64_t *)(buffer.data() + 0x0a));
    int64_t maybeInhabitedTime = mcfile::I64FromBE(*(int64_t *)(buffer.data() + 0x12));
    cout << "cx: " << xPos << ", cz: " << zPos << endl;

    uint8_t maybeMinSectionY = buffer[0x1a]; // Always 0. May be > 0 in The End?
    cout << "minSectionY: " << (int)maybeMinSectionY << endl;

    uint16_t maxJumpTableAddress = (uint16_t)buffer[0x1b] * 0x100;
    cout << "maxJumpTableAddress: 0x" << hex << maxJumpTableAddress << dec << endl;
    vector<uint16_t> maybeJumpTableFor16Sections;
    for (int section = 0; section < 16; section++) {
      uint16_t address = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + 0x1c + section * sizeof(uint16_t)));
      maybeJumpTableFor16Sections.push_back(address);
    }

    vector<uint8_t> maybeNumBlockPaletteEntriesFor16Sections;
    for (int section = 0; section < 16; section++) {
      uint8_t numBlockPaletteEntries = buffer[0x3c + section];
      maybeNumBlockPaletteEntriesFor16Sections.push_back(numBlockPaletteEntries);
    }

    for (int section = 0; section < 16; section++) {
      uint16_t address = maybeJumpTableFor16Sections[section];
      uint8_t paletteSize = maybeNumBlockPaletteEntriesFor16Sections[section];
      cout << "section#" << section << " at 0x" << hex << address << dec;
      if (address == maxJumpTableAddress) {
        cout << " (empty)";
        CHECK(paletteSize == 0);
      } else {
        cout << ", block palette: " << (int)paletteSize << " blocks";
      }
      cout << endl;
    }

    vector<uint8_t> unknown128BytesA;
    copy_n(buffer.data() + 0x4c, 128, back_inserter(unknown128BytesA)); // [0x4c, 0xcb]
    /*
    after:  00 F0 00 00 00 00 00 00 40 F0 00 00 00 00 00 00 80 F0 00 00 00 00 00 00 C0 F0 00 00 00 00 00 00 00 41 00 00 00 00 00 00 06 81 00 00 00 00 00 00 16 61 00 00 00 00 00 00 20 81 00 00 00 00 00 00 30 41 00 00 00 00 00 00 36 21 00 00 00 00 00 00 39 21 00 00 00 00 00 00 3C 21 00 00 00 00 00 00 3F F1 00 00 00 00 00 00 7F 81 00 00 00 00 00 00 8F 21 00 00 00 00 00 00 92 21 00 00 00 00 00 00
    before: 00 F0 00 00 00 00 00 00 40 F0 00 00 00 00 00 00 80 80 00 00 00 00 00 00 90 F0 00 00 00 00 00 00 D0 40 00 00 00 00 00 00 D6 80 00 00 00 00 00 00 E6 60 00 00 00 00 00 00 F0 80 00 00 00 00 00 00 00 41 00 00 00 00 00 00 06 21 00 00 00 00 00 00 09 21 00 00 00 00 00 00 0C 21 00 00 00 00 00 00 0F F1 00 00 00 00 00 00 4F 81 00 00 00 00 00 00 5F 21 00 00 00 00 00 00 62 21 00 00 00 00 00 00
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    diff                                                       70                   30                      xx 01                   xx 01                   xx 01                   xx 01                   30                      30                      30                      30                      30                      30                      30                      30
                                                            8080                    F009                    4100 - 40D0 = 30        8106 - 80D6 = 30        6116 - 60e6 = 30        8120 - 80f0 = 30        4100                    2106                    2109                    210C                    F10F                    814F                    215F                    2162
    (b)3012 000F                    040F                    0808                    090F                    0D04                    0D68                    0E66                    0F08                    1004                    1062                    1092                    10C2                    10FF                    14F8                    15F2                    1622
    (b)3021 00F0                    04F0                    0880                    09F0                    0D40                    0D86                    0E66                    0F80                    1040                    1026                    1029                    102C                    10FF                    148F                    152F                    1622
    (b)1302 000F                    004F                    0088                    009F                    00D4                    60D8                    60E6                    00F8
    (b)2301 F000                    F040                    8080                    F090                    40D0                    80D6                    60E6                    80F0
    (b)1230 0F00                    0F04                    0808                    0F09                    040D                    680D                    660E                    080F
    (b)2130 F000                    F004                    8008                    F009                    400D                    860D                    660E                    800F

    When selecting digits in order 3012, the uint16_t values increase monotonic.

    Exceptions:
    "2-Save20220306005722-000-fill-bedrock-under-sea-level.bin"
    00 40 06 40 10 00 0C 20 0F 40 15 40 10 00 10 00 1B 60 25 40 2B 40 31 20 34 40 3A 20 3D 40 43 20 46 40 4C 40 52 40 58 20 5B 40 61 40 67 40 10 00 6D 40 73 40 79 40 10 00 7F 40 10 00 10 00 10 00 85 40 8B 40 91 40 97 40 9D 40 A3 40 A9 20 AC 20 AF 40 B5 20 B8 40 BE 20 C1 20 C4 40 CA 40 10 00 D0 40 10 00 D6 40 DC 40 E2 40 E8 20 10 00 EB 20 EE 20 F1 20 F4 20 F7 20 FA 40 00 61 0A 61 14 41

    "2-Save20220306005722-001-chunk-filled-by-bedrock.bin"
    00 40 06 40 10 00 0C 20 0F 40 15 40 10 00 10 00 1B 60 25 40 2B 40 31 20 34 40 3A 20 3D 40 43 20 46 40 4C 40 52 40 58 20 5B 40 61 40 67 40 10 00 6D 40 73 40 79 40 10 00 7F 40 10 00 10 00 10 00 85 40 8B 40 91 40 97 40 9D 40 A3 40 A9 20 AC 20 AF 40 B5 20 B8 40 BE 20 C1 20 C4 40 CA 40 10 00 D0 40 10 00 D6 40 DC 40 E2 40 E8 20 10 00 EB 20 EE 20 F1 20 F4 20 F7 20 FA 40 00 61 0A 61 14 41
    */
    uint16_t prev = 0;
    for (int i = 0; i < 64; i++) {
      uint8_t v1 = unknown128BytesA[i * 2];
      uint8_t v2 = unknown128BytesA[i * 2 + 1];
      uint8_t t1 = v1 >> 4;
      uint8_t t2 = 0xf & v1;
      uint8_t t3 = v2 >> 4;
      uint8_t t4 = 0xf & v2;
      uint16_t address = t4 << 12 | t1 << 8 | t2 << 4 | t3;
      if (address != 0) {
        CHECK(address >= prev);
        prev = address;
      }
      cout << "#" << i << hex << ": 0x" << (int)address << dec << endl;
    }

    vector<uint8_t> section0;
    copy_n(buffer.data() + 0xcc, 128, back_inserter(section0)); // [0xcc, 0x14b]
    for (int x = 0; x < 4; x++) {
      for (int z = 0; z < 4; z++) {
        for (int y = 0; y < 4; y++) {
          int position = 2 * (16 * x + 4 * z + y);
          uint8_t v1 = section0[position];
          uint8_t v2 = section0[position + 1];

          uint8_t t1 = v1 >> 4;
          uint8_t t2 = 0xf & v1;
          uint8_t t3 = v2 >> 4;
          uint8_t t4 = 0xf & v2;

          uint16_t blockId = t4 << 4 | t1;
          uint8_t data = t3 << 4 | t2;
          auto block = mcfile::je::Flatten::DoFlatten(blockId, data);
          //cout << "[" << x << ", " << y << ", " << z << "] = " << block->toString() << endl;
        }
      }
    }

    // [0, 1, 4] = 0x1CE
    // [4, 1, 0] = 0x410???
    // [0, 1, 8] = 0x2CE
    // [0, 1, 12] = 0x3CE
    // [4, 1, 0] = 0x4D2

    // Biomes: 256 bytes / chunk
    // HeightMap: 256 bytes / chunk

    // BlockLight: 2048 bytes / section: 4bit per block
    // Blocks: 4096 bytes / section: 1byte per block
    // Data: 2048 bytes / section: 4bit per block
    // SkyLight: 2048 bytes / section: 4bit per block
    // --
    // Total: 10240 bytes / section

    int heightMapStartPos = buffer.size() - 256 - 2 - 256;

    /*
    sectionsTotalSize=6544 bytes
    sectionsTotalSize=17040 bytes
    sectionsTotalSize=22288 bytes
    sectionsTotalSize=75408 bytes
    */

    vector<uint8_t> heightMap;
    copy_n(buffer.data() + heightMapStartPos, 256, back_inserter(heightMap)); // When heightMap[n] == 0, it means height = 256 at n.

    cout << "height map:" << endl;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        int h = heightMap[x + z * 16];
        if (h == 0) {
          h = 256;
        }
        printf("%3d ", (int)h);
      }
      printf("\n");
    }

    uint16_t unknownMarkerA = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + buffer.size() - 256 - 2)); // 0x07fe
    CHECK(unknownMarkerA == 0x07fe);

    vector<mcfile::biomes::BiomeId> biomes;
    for (int i = 0; i < 256; i++) {
      auto raw = buffer[buffer.size() - 256 - 1];
      auto biome = mcfile::biomes::FromInt(raw);
      biomes.push_back(biome);
    }
  }
}

#if 1
TEST_CASE("research") {
  Box360Chunk();
}
#endif
