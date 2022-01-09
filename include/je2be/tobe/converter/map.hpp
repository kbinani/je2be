#pragma once

namespace je2be::tobe {

class Map {
  using CompoundTag = mcfile::nbt::CompoundTag;

public:
  static int64_t UUID(int32_t javaMapId, uint8_t scale) {
    uint32_t const seed = 0;
    uint32_t hash = XXHash32::hash(&javaMapId, sizeof(javaMapId), 0);
    uint64_t s = (uint64_t)hash * 10 + (4 - scale);
    return *(int64_t *)&s;
  }

  static bool Convert(int32_t javaMapId, CompoundTag const &item, std::filesystem::path const &input, InputOption const &opt, DbInterface &db) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile::stream;
    using namespace props;
    using namespace mcfile::nbt;

    auto root = JavaEditionMap::Read(input, opt, javaMapId);
    if (!root) {
      return false;
    }

    auto data = root->query("/data")->asCompound();
    if (!data) {
      return false;
    }

    auto dimensionString = data->string("dimension");
    auto dimensionInt = data->int32("dimension");
    auto dimensionByte = data->byte("dimension");

    int8_t outDimension = 0;
    if (dimensionString) {
      if (*dimensionString == "minecraft:overworld") {
        outDimension = 0;
      } else if (*dimensionString == "minecraft:the_nether") {
        outDimension = 1;
      } else if (*dimensionString == "minecraft:the_end") {
        outDimension = 2;
      }
    } else if (dimensionInt) {
      // <= 1.15
      outDimension = *dimensionInt;
    } else if (dimensionByte) {
      outDimension = *dimensionByte;
    }
    auto locked = data->boolean("locked", false);
    auto scale = data->byte("scale");
    auto xCenter = data->int32("xCenter");
    auto zCenter = data->int32("zCenter");
    auto colors = data->byteArrayTag("colors");
    auto unlimitedTracking = data->boolean("unlimitedTracking", false);

    if (!scale || !xCenter || !zCenter || !colors) {
      return false;
    }

    for (uint8_t beScale = 0; beScale <= 4; beScale++) {
      int64_t uuid = UUID(javaMapId, beScale);
      auto ret = make_shared<CompoundTag>();
      ret->set("dimension", Byte(outDimension));
      ret->set("fullyExplored", Bool(false)); //?
      ret->set("height", Short(128));
      ret->set("mapId", Long(uuid));
      ret->set("mapLocked", Bool(locked));
      if (beScale == 4) {
        ret->set("parentMapId", Long(-1));
      } else {
        int64_t parent = UUID(javaMapId, beScale + 1);
        ret->set("parentMapId", Long(parent));
      }
      ret->set("scale", Byte(beScale));
      ret->set("unlimitedTracking", Bool(unlimitedTracking));
      ret->set("width", Short(128));
      ret->set("xCenter", Int(*xCenter));
      ret->set("zCenter", Int(*zCenter));

      std::vector<uint8_t> outColors(65536);
      auto decorations = make_shared<ListTag>(Tag::Type::Compound);

      if (beScale == *scale) {
        int i = 0;
        int j = 0;
        vector<uint8_t> const &colorsArray = colors->value();
        for (int y = 0; y < 128; y++) {
          for (int x = 0; x < 128; x++, i++) {
            uint8_t colorId = colorsArray[i];
            Rgba color = RgbaFromId(colorId);
            outColors[j] = color.fR;
            outColors[j + 1] = color.fG;
            outColors[j + 2] = color.fB;
            outColors[j + 3] = color.fA;
            j += 4;
          }
        }

        auto frames = data->listTag("frames");
        if (frames) {
          for (auto const &it : *frames) {
            auto frame = it->asCompound();
            if (!frame) {
              continue;
            }
            auto pos = frame->compoundTag("Pos");
            auto rotation = frame->int32("Rotation");
            if (!pos || !rotation) {
              continue;
            }
            auto x = pos->int32("X");
            auto y = pos->int32("Y");
            auto z = pos->int32("Z");
            if (!x || !y || !z) {
              continue;
            }

            int32_t rot = 0;
            if (*rotation == -90) {
              rot = 0; // top; arrow facing to east in Java, arrow facing to
                       // south in Bedrock
            } else if (*rotation == 180) {
              rot = 8; // north; arrow facing to north
            } else if (*rotation == 270) {
              rot = 12; // east; arrow facing to east
            } else if (*rotation == 0) {
              rot = 0; // south; arrow facing to south
            } else {
              // nop
              // west; *rotation == -90; arrow facing to west
              // bottom; *rotation == -90; arrow facing to east in Java, arrow
              // facing to south in Bedrock
            }

            auto frameData = make_shared<CompoundTag>();
            frameData->set("rot", Int(rot));
            frameData->set("type", Int(1));
            auto [markerX, markerY] = MarkerPosition(*x, *z, *xCenter, *zCenter, *scale);
            if (markerX < -128 || 128 < markerX || markerY < -128 || 128 < markerY) {
              continue;
            }
            frameData->set("x", Int(markerX));
            frameData->set("y", Int(markerY));

            auto key = make_shared<CompoundTag>();
            key->set("blockX", Int(*x));
            key->set("blockY", Int(*y));
            key->set("blockZ", Int(*z));
            key->set("type", Int(1));

            auto decoration = make_shared<CompoundTag>();
            decoration->set("data", frameData);
            decoration->set("key", key);

            decorations->push_back(decoration);
          }
        }

        auto inDecorations = item.query("tag/Decorations")->asList();
        if (inDecorations) {
          for (auto const &d : *inDecorations) {
            auto e = d->asCompound();
            if (!e) {
              continue;
            }
            auto type = e->byte("type");
            auto x = e->float64("x");
            auto z = e->float64("z");
            if (!type || !x || !z) {
              continue;
            }
            int32_t outType = 1;
            if (*type == 9) {
              outType = 15; // id = "+", monument
            } else if (*type == 8) {
              outType = 14; // id = "+", mansion
            } else if (*type == 26) {
              outType = 4; // id = "+", buried treasure
            }

            auto frameData = make_shared<CompoundTag>();
            frameData->set("rot", Int(8));
            frameData->set("type", Int(outType));
            auto [markerX, markerY] = MarkerPosition(*x, *z, *xCenter, *zCenter, *scale);
            if (markerX < -128 || 128 < markerX || markerY < -128 || 128 < markerY) {
              continue;
            }
            frameData->set("x", Int(markerX));
            frameData->set("y", Int(markerY));

            auto key = make_shared<CompoundTag>();
            key->set("blockX", Int((int32_t)*x));
            key->set("blockZ", Int((int32_t)*z));
            key->set("blockY", Int(64)); // fixed value?
            key->set("type", Int(1));    //?

            auto decoration = make_shared<CompoundTag>();
            decoration->set("data", frameData);
            decoration->set("key", key);

            decorations->push_back(decoration);
          }
        }
      }
      auto outColorsTag = make_shared<ByteArrayTag>(outColors);
      ret->set("colors", outColorsTag);

      ret->set("decorations", decorations);

      vector<uint8_t> serialized;
      {
        auto out = make_shared<ByteStream>();
        OutputStreamWriter w(out, {.fLittleEndian = true});
        if (!ret->writeAsRoot(w)) {
          return false;
        }
        out->drain(serialized);
      }
      auto key = mcfile::be::DbKey::Map(uuid);
      db.put(key, leveldb::Slice((char *)serialized.data(), serialized.size()));
    }

    return true;
  }

private:
  static std::tuple<int32_t, int32_t> MarkerPosition(int32_t blockX, int32_t blockZ, int32_t xCenter, int32_t zCenter, int32_t scale) {
    int32_t size = 128 * (int32_t)ceil(pow(2, scale));
    int32_t x0 = xCenter - size / 2;
    int32_t z0 = zCenter - size / 2;
    /*
     * marker (-128, -128) corresponds to block (x0, z0)
     * marker (128, 128) corresponds to block (x0 + size, z0 + size)
     */
    int32_t markerX = (blockX - xCenter) * 256 / size;
    int32_t markerY = (blockZ - zCenter) * 256 / size;
    return std::make_tuple(markerX, markerY);
  }

  static Rgba RgbaFromId(uint8_t colorId) {
    static std::vector<Rgba> const mapping = {
        Rgba(0, 0, 0, 0),
        Rgba(127, 178, 56),
        Rgba(247, 233, 163),
        Rgba(199, 199, 199),
        Rgba(255, 0, 0),
        Rgba(160, 160, 255),
        Rgba(167, 167, 167),
        Rgba(0, 124, 0),
        Rgba(255, 255, 255),
        Rgba(164, 168, 184),
        Rgba(151, 109, 77),
        Rgba(112, 112, 112),
        Rgba(64, 64, 255),
        Rgba(143, 119, 72),
        Rgba(255, 252, 245),
        Rgba(216, 127, 51),
        Rgba(178, 76, 216),
        Rgba(102, 153, 216),
        Rgba(229, 229, 51),
        Rgba(127, 204, 25),
        Rgba(242, 127, 165),
        Rgba(76, 76, 76),
        Rgba(153, 153, 153),
        Rgba(76, 127, 153),
        Rgba(127, 63, 178),
        Rgba(51, 76, 178),
        Rgba(102, 76, 51),
        Rgba(102, 127, 51),
        Rgba(153, 51, 51),
        Rgba(25, 25, 25),
        Rgba(250, 238, 77),
        Rgba(92, 219, 213),
        Rgba(74, 128, 255),
        Rgba(0, 217, 58),
        Rgba(129, 86, 49),
        Rgba(112, 2, 0),
        Rgba(209, 177, 161),
        Rgba(159, 82, 36),
        Rgba(149, 87, 108),
        Rgba(112, 108, 138),
        Rgba(186, 133, 36),
        Rgba(103, 117, 53),
        Rgba(160, 77, 78),
        Rgba(57, 41, 35),
        Rgba(135, 107, 98),
        Rgba(87, 92, 92),
        Rgba(122, 73, 88),
        Rgba(76, 62, 92),
        Rgba(76, 50, 35),
        Rgba(76, 82, 42),
        Rgba(142, 60, 46),
        Rgba(37, 22, 16),
        Rgba(189, 48, 49),
        Rgba(148, 63, 97),
        Rgba(92, 25, 29),
        Rgba(22, 126, 134),
        Rgba(58, 142, 140),
        Rgba(86, 44, 62),
        Rgba(20, 180, 133),
    };
    uint8_t variant = 0x3 & colorId;
    uint8_t index = colorId / 4;
    if (index >= mapping.size()) {
      return mapping[0];
    }
    Rgba base = mapping[index];
    static int32_t const mul[4] = {180, 220, 255, 135};
    uint8_t r = (uint8_t)((int32_t)base.fR * mul[variant] / 255);
    uint8_t g = (uint8_t)((int32_t)base.fG * mul[variant] / 255);
    uint8_t b = (uint8_t)((int32_t)base.fB * mul[variant] / 255);
    return Rgba(r, g, b, base.fA);
  }

private:
  Map() = delete;
};

} // namespace je2be::tobe