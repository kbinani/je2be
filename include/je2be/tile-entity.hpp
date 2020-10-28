#pragma once

namespace j2b {

class TileEntity {
public:
    using TileEntityData = std::shared_ptr<mcfile::nbt::CompoundTag>;

private:
    using CompoundTag = mcfile::nbt::CompoundTag;
    using Block = mcfile::Block;
    using Converter = std::function<TileEntityData(Pos const&, Block const&, std::shared_ptr<CompoundTag> const&)>;

public:
    static bool IsTileEntity(std::string const& name) {
        auto const& table = Table();
        auto found = table.find(name);
        return found != table.end();
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> From(Pos const& pos, mcfile::Block const& block, std::shared_ptr<mcfile::nbt::CompoundTag> const& tag) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;

        string const& name = block.fName;
        auto const& table = Table();
        return table.at(name)(pos, block, tag);
    }

private:
    TileEntity() = delete;

    static std::unordered_map<std::string, Converter> const& Table() {
        using namespace std;
        static unique_ptr<unordered_map<string, Converter> const> const table(CreateTable());
        return *table;
    }

    static std::unordered_map<std::string, Converter>* CreateTable() {
        using namespace std;
        auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
        E("chest", Chest);
        E("trapped_chest", Chest);
        E("oak_sign", Sign);
        E("spruce_sign", Sign);
        E("birch_sign", Sign);
        E("jungle_sign", Sign);
        E("acacia_sign", Sign);
        E("dark_oak_sign", Sign);
        E("crimson_sign", Sign);
        E("warped_sign", Sign);
        E("oak_wall_sign", Sign);
        E("spruce_wall_sign", Sign);
        E("birch_wall_sign", Sign);
        E("jungle_wall_sign", Sign);
        E("acacia_wall_sign", Sign);
        E("dark_oak_wall_sign", Sign);
        E("crimson_wall_sign", Sign);
        E("warped_wall_sign", Sign);

        E("shulker_box", ShulkerBox);
        E("black_shulker_box", ShulkerBox);
        E("red_shulker_box", ShulkerBox);
        E("green_shulker_box", ShulkerBox);
        E("brown_shulker_box", ShulkerBox);
        E("blue_shulker_box", ShulkerBox);
        E("purple_shulker_box", ShulkerBox);
        E("cyan_shulker_box", ShulkerBox);
        E("light_gray_shulker_box", ShulkerBox);
        E("gray_shulker_box", ShulkerBox);
        E("pink_shulker_box", ShulkerBox);
        E("lime_shulker_box", ShulkerBox);
        E("yellow_shulker_box", ShulkerBox);
        E("light_blue_shulker_box", ShulkerBox);
        E("magenta_shulker_box", ShulkerBox);
        E("orange_shulker_box", ShulkerBox);
        E("white_shulker_box", ShulkerBox);

        E("white_bed", Bed);
        E("orange_bed", Bed);
        E("magenta_bed", Bed);
        E("light_blue_bed", Bed);
        E("yellow_bed", Bed);
        E("lime_bed", Bed);
        E("pink_bed", Bed);
        E("gray_bed", Bed);
        E("light_gray_bed", Bed);
        E("cyan_bed", Bed);
        E("purple_bed", Bed);
        E("blue_bed", Bed);
        E("brown_bed", Bed);
        E("green_bed", Bed);
        E("red_bed", Bed);
        E("black_bed", Bed);

        E("white_banner", Banner);
        E("orange_banner", Banner);
        E("magenta_banner", Banner);
        E("light_blue_banner", Banner);
        E("yellow_banner", Banner);
        E("lime_banner", Banner);
        E("pink_banner", Banner);
        E("gray_banner", Banner);
        E("light_gray_banner", Banner);
        E("cyan_banner", Banner);
        E("purple_banner", Banner);
        E("blue_banner", Banner);
        E("brown_banner", Banner);
        E("green_banner", Banner);
        E("red_banner", Banner);
        E("black_banner", Banner);

        E("white_wall_banner", Banner);
        E("orange_wall_banner", Banner);
        E("magenta_wall_banner", Banner);
        E("light_blue_wall_banner", Banner);
        E("yellow_wall_banner", Banner);
        E("lime_wall_banner", Banner);
        E("pink_wall_banner", Banner);
        E("gray_wall_banner", Banner);
        E("light_gray_wall_banner", Banner);
        E("cyan_wall_banner", Banner);
        E("purple_wall_banner", Banner);
        E("blue_wall_banner", Banner);
        E("brown_wall_banner", Banner);
        E("green_wall_banner", Banner);
        E("red_wall_banner", Banner);
        E("black_wall_banner", Banner);

        E("potted_oak_sapling", PottedSapling);
        E("potted_spruce_sapling", PottedSapling);
        E("potted_birch_sapling", PottedSapling);
        E("potted_jungle_sapling", PottedSapling);
        E("potted_acacia_sapling", PottedSapling);
        E("potted_dark_oak_sapling", PottedSapling);
        E("potted_poppy", PottedPlant("red_flower", {{"flower_type", "poppy"}}));
        E("potted_blue_orchid", PottedPlant("red_flower", {{"flower_type", "orchid"}}));
        E("potted_allium", PottedPlant("red_flower", {{"flower_type", "allium"}}));
        E("potted_azure_bluet", PottedPlant("red_flower", {{"flower_type", "houstonia"}}));
        E("potted_red_tulip", PottedPlant("red_flower", {{"flower_type", "tulip_red"}}));
        E("potted_orange_tulip", PottedPlant("red_flower", {{"flower_type", "tulip_orange"}}));
        E("potted_white_tulip", PottedPlant("red_flower", {{"flower_type", "tulip_white"}}));
        E("potted_pink_tulip", PottedPlant("red_flower", {{"flower_type", "tulip_pink"}}));
        E("potted_oxeye_daisy", PottedPlant("red_flower", {{"flower_type", "oxeye"}}));
        E("potted_cornflower", PottedPlant("red_flower", {{"flower_type", "cornflower"}}));
        E("potted_lily_of_the_valley", PottedPlant("red_flower", {{"flower_type", "lily_of_the_valley"}}));
        E("potted_dandelion", PottedPlant("yellow_flower", {}));
        E("potted_wither_rose", PottedPlant("wither_rose", {}));
        E("potted_crimson_fungus", PottedPlant("crimson_fungus", {}));
        E("potted_warped_fungus", PottedPlant("warped_fungus", {}));
        E("potted_dead_bush", PottedPlant("deadbush", {}));
        E("potted_red_mushroom", PottedPlant("red_mushroom", {}));
        E("potted_brown_mushroom", PottedPlant("brown_mushroom", {}));
        E("potted_fern", PottedPlant("tallgrass", {{"tall_grass_type", "fern"}}));
        E("potted_bamboo", PottedBamboo);
        E("potted_crimson_roots", PottedPlant("crimson_roots", {}));
        E("potted_warped_roots", PottedPlant("warped_roots", {}));

        E("skeleton_skull", Skull);
        E("wither_skeleton_skull", Skull);
        E("player_head", Skull);
        E("zombie_head", Skull);
        E("creeper_head", Skull);
        E("dragon_head", Skull);
#undef E
        return table;
    }

    static TileEntityData Skull(Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;
        auto tag = make_shared<CompoundTag>();
        int8_t type = 0;
        auto const& name = b.fName;
        if (name == "minecraft:player_head") {
            type = 3;
        } else if (name == "minecraft:zombie_head") {
            type = 2;
        } else if (name == "minecraft:creeper_head") {
            type = 4;
        } else if (name == "minecraft:dragon_head") {
            type = 5;
        } else if (name == "minecraft:skeleton_skull") {
            type = 0;
        } else if (name == "minecraft:wither_skeleton_skull") {
            type = 1;
        }
        auto rot = stoi(b.property("rotation", "0"));
        float rotation = rot / 16.0f * 360.0f;
        tag->fValue = {
            {"id", String("Skull")},
            {"isMovable", Bool(true)},
            {"MouthMoving", Bool(false)},
            {"MouthTickCount", Int(0)},
            {"SkullType", Byte(type)},
            {"Rotation", Float(rotation)},
        };
        Attach(pos, *tag);
        return tag;
    }

    static TileEntityData PottedBamboo(Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto tag = make_shared<CompoundTag>();
        auto plantBlock = make_shared<CompoundTag>();
        auto states = make_shared<CompoundTag>();
        states->fValue = {
            {"age_bit", Byte(0)},
            {"bamboo_leaf_size", String("no_leaves")},
            {"mamboo_stalk_thickness", String("thin")},
        };
        plantBlock->fValue = {
            {"states", states},
            {"name", String("minecraft:bamboo")},
            {"version", Int(BlockData::kBlockDataVersion)},
        };
        tag->fValue = {
            {"id", String("FlowerPot")},
            {"isMovable", Bool(true)},
            {"PlantBlock", plantBlock},
        };
        Attach(pos, *tag);
        return tag;
    }

    static Converter PottedPlant(std::string const& name, std::map<std::string, std::string> const& properties) {
        return [=](Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) -> TileEntityData {
            using namespace props;
            using namespace mcfile::nbt;
            using namespace std;

            auto tag = make_shared<CompoundTag>();
            auto plantBlock = make_shared<CompoundTag>();
            auto states = make_shared<CompoundTag>();
            for (auto const& p : properties) {
                states->fValue.emplace(p.first, String(p.second));
            }
            plantBlock->fValue = {
                {"states", states},
                {"name", String("minecraft:" + name)},
                {"version", Int(BlockData::kBlockDataVersion)},
            };
            tag->fValue = {
                {"id", String("FlowerPot")},
                {"isMovable", Bool(true)},
                {"PlantBlock", plantBlock},
            };
            Attach(pos, *tag);
            return tag;
        };
    }

    static TileEntityData PottedSapling(Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto tag = make_shared<CompoundTag>();
        auto plantBlock = make_shared<CompoundTag>();
        auto states = make_shared<CompoundTag>();
        auto type = strings::RTrim(strings::LTrim(b.fName, "minecraft:potted_"), "_sapling");
        states->fValue = {
            {"age_bit", Byte(0)},
            {"sapling_type", String(type)},
        };
        plantBlock->fValue = {
            {"states", states},
            {"name", String("minecraft:sapling")},
            {"version", Int(BlockData::kBlockDataVersion)},
        };
        tag->fValue = {
            {"id", String("FlowerPot")},
            {"isMovable", Bool(true)},
            {"PlantBlock", plantBlock},
        };
        Attach(pos, *tag);
        return tag;
    }

    static TileEntityData Banner(Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        using namespace mcfile::nbt;
        auto tag = std::make_shared<CompoundTag>();

        auto customName = GetJson(*c, "CustomName");
        int32_t type = 0;
        if (customName) {
            auto color = GetAsString(*customName, "color");
            auto translate = GetAsString(*customName, "translate");
            if (color == "gold" && translate == "block.minecraft.ominous_banner") {
                type = 1; // Illager Banner
            }
        }

        auto patterns = GetList(*c, "Patterns");
        auto patternsBedrock = std::make_shared<ListTag>();
        patternsBedrock->fType = Tag::TAG_Compound;
        if (patterns && type != 1) {
            for (auto const& pattern : patterns->fValue) {
                auto p = pattern->asCompound();
                if (!p) continue;
                auto color = GetInt(*p, "Color");
                auto pat = GetString(*p, "Pattern");
                if (!color || !pat) continue;
                auto ptag = std::make_shared<CompoundTag>();
                ptag->fValue = {
                    {"Color", Int(BannerColorCodeFromJava(*color))},
                    {"Pattern", String(*pat)},
                };
                patternsBedrock->fValue.push_back(ptag);
            }
        }

        int32_t base = BannerColor(b.fName);

        tag->fValue = {
            {"id", String("Banner")},
            {"isMovable", Bool(true)},
            {"Type", Int(type)},
            {"Base", Int(base)},
            {"Patterns", patternsBedrock},
        };
        Attach(pos, *tag);
        return tag;
    }

    static int32_t BannerColorCodeFromJava(int32_t java) {
        ColorCodeJava ccj = (ColorCodeJava)java;
        switch (ccj) {
        case ColorCodeJava::Red:
            return (int32_t)BannerColorCodeBedrock::Red;
        case ColorCodeJava::Black:
            return (int32_t)BannerColorCodeBedrock::Black;
        case ColorCodeJava::Blue:
            return (int32_t)BannerColorCodeBedrock::Blue;
        case ColorCodeJava::Brown:
            return (int32_t)BannerColorCodeBedrock::Brown;
        case ColorCodeJava::Cyan:
            return (int32_t)BannerColorCodeBedrock::Cyan;
        case ColorCodeJava::Gray:
            return (int32_t)BannerColorCodeBedrock::Gray;
        case ColorCodeJava::Green:
            return (int32_t)BannerColorCodeBedrock::Green;
        case ColorCodeJava::LightBlue:
            return (int32_t)BannerColorCodeBedrock::LightBlue;
        case ColorCodeJava::LightGray:
            return (int32_t)BannerColorCodeBedrock::LightGray;
        case ColorCodeJava::Lime:
            return (int32_t)BannerColorCodeBedrock::Lime;
        case ColorCodeJava::Magenta:
            return (int32_t)BannerColorCodeBedrock::Magenta;
        case ColorCodeJava::Orange:
            return (int32_t)BannerColorCodeBedrock::Orange;
        case ColorCodeJava::Pink:
            return (int32_t)BannerColorCodeBedrock::Pink;
        case ColorCodeJava::Purple:
            return (int32_t)BannerColorCodeBedrock::Purple;
        case ColorCodeJava::White:
            return (int32_t)BannerColorCodeBedrock::White;
        case ColorCodeJava::Yellow:
            return (int32_t)BannerColorCodeBedrock::Yellow;
        }
    }

    static int32_t BannerColor(std::string const& name) {
        auto color = name.substr(10); // minecraft:
        auto suffix = color.rfind("_wall_banner");
        if (suffix != std::string::npos) {
            color = color.substr(0, suffix);
        }
        suffix = color.rfind("_banner");
        if (suffix != std::string::npos) {
            color = color.substr(0, suffix);
        }
        static std::unordered_map<std::string, int32_t> const mapping = {
            {"black", 0},
            {"red", 1},
            {"green", 2},
            {"brown", 3},
            {"blue", 4},
            {"purple", 5},
            {"cyan", 6},
            {"light_gray", 7},
            {"gray", 8},
            {"pink", 9},
            {"lime", 10},
            {"yellow", 11},
            {"light_blue", 12},
            {"magenta", 13},
            {"orange", 14},
            {"white", 15},
        };
        auto found = mapping.find(color);
        if (found == mapping.end()) {
            return 0;
        }
        return found->second;
    }

    static int8_t BedColor(std::string const& name) {
        static std::unordered_map<std::string, int8_t> const mapping = {
            {"minecraft:white_bed", 0},
            {"minecraft:orange_bed", 1},
            {"minecraft:magenta_bed", 2},
            {"minecraft:light_blue_bed", 3},
            {"minecraft:yellow_bed", 4},
            {"minecraft:lime_bed", 5},
            {"minecraft:pink_bed", 6},
            {"minecraft:gray_bed", 7},
            {"minecraft:light_gray_bed", 8},
            {"minecraft:cyan_bed", 9},
            {"minecraft:purple_bed", 10},
            {"minecraft:blue_bed", 11},
            {"minecraft:brown_bed", 12},
            {"minecraft:green_bed", 13},
            {"minecraft:red_bed", 14},
            {"minecraft:black_bed", 15},
        };
        auto found = mapping.find(name);
        if (found == mapping.end()) {
            return 0;
        }
        return found->second;
    }

    static TileEntityData Bed(Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        auto tag = std::make_shared<CompoundTag>();
        auto color = BedColor(b.fName);
        tag->fValue = {
            {"id", String("Bed")},
            {"color", Byte(color)},
            {"isMovable", Bool(true)},
        };
        Attach(pos, *tag);
        return tag;
    }

    static TileEntityData ShulkerBox(Pos const& pos, Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        auto facing = BlockData::GetFacingDirectionFromFacingA(b);
        auto items = GetItems(*c, "Items");
        auto tag = std::make_shared<CompoundTag>();
        tag->fValue = {
            {"id", String("ShulkerBox")},
            {"facing", Byte((int8_t)facing)},
            {"Findable", Bool(false)},
            {"isMovable", Bool(true)},
            {"Items", items},
        };
        Attach(pos, *tag);
        return tag;
    }

    static std::string ColorCode(std::string const& color) {
        if (color == "black") {
            return "§0";
        } else if (color == "red") {
            return "§4";
        } else if (color == "green") {
            return "§2";
        } else if (color == "brown") {
            return ""; // no matching color for brown
        } else if (color == "blue") {
            return "§1";
        } else if (color == "purple") {
            return "§5";
        } else if (color == "cyan") {
            return "§3";
        } else if (color == "light_gray") {
            return "§7";
        } else if (color == "gray") {
            return "§8";
        } else if (color == "pink") {
            return "§d"; // not best match. same as magenta
        } else if (color == "lime") {
            return "§a";
        } else if (color == "yellow") {
            return "§e";
        } else if (color == "light_blue") {
            return "§b";
        } else if (color == "magenta") {
            return "§d";
        } else if (color == "orange") {
            return "§6";
        } else if (color == "white") {
            return "§f";
        }
        return "";
    }

    static std::string GetAsString(nlohmann::json const& obj, std::string const& key) {
        auto found = obj.find(key);
        if (found == obj.end()) return "";
        return found->get<std::string>();
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Sign(Pos const& pos, mcfile::Block const& b, std::shared_ptr<CompoundTag> const& c) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto color = GetString(*c, "Color");
        auto text1 = GetJson(*c, "Text1");
        auto text2 = GetJson(*c, "Text2");
        auto text3 = GetJson(*c, "Text3");
        auto text4 = GetJson(*c, "Text4");
        if (!color || !text1 || !text2 || !text3 || !text4) return nullptr;
        string text = "";
        if (*color != "black") {
            text += ColorCode(*color);
        }
        text += GetAsString(*text1, "text") + "\x0a" + GetAsString(*text2, "text") + "\x0a" + GetAsString(*text3, "text") + "\x0a" + GetAsString(*text4, "text");
        auto tag = make_shared<CompoundTag>();
        tag->fValue = {
            {"id", String("Sign")},
            {"isMovable", Bool(true)},
            {"Text", String(text)},
            {"TextOwner", String("")},
        };
        Attach(pos, *tag);
        return tag;
    }

    static void Attach(Pos const& pos, mcfile::nbt::CompoundTag & tag) {
        tag.fValue.insert(std::make_pair("x", props::Int(pos.fX)));
        tag.fValue.insert(std::make_pair("y", props::Int(pos.fY)));
        tag.fValue.insert(std::make_pair("z", props::Int(pos.fZ)));
    }

    static std::shared_ptr<mcfile::nbt::ListTag> GetItems(mcfile::nbt::CompoundTag const& c, std::string const& name) {
        auto tag = std::make_shared<mcfile::nbt::ListTag>();
        tag->fType = mcfile::nbt::Tag::TAG_Compound;
        auto list = GetList(c, name);
        if (list == nullptr) {
            return tag;
        }
        //TODO:
        return tag;
    }

    static mcfile::nbt::ListTag const* GetList(CompoundTag const& c, std::string const& name) {
        auto found = c.fValue.find(name);
        if (found == c.fValue.end()) {
            return nullptr;
        }
        return found->second->asList();
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Chest(Pos const& pos, mcfile::Block const& b, std::shared_ptr<CompoundTag> const& comp) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto type = b.property("type", "single");
        auto facing = b.property("facing", "north");
        optional<pair<int, int>> pair;
        if (type == "left" && facing == "north") {
            pair = make_pair(pos.fX + 1, pos.fZ);
        } else if (type == "right" && facing == "south") {
            pair = make_pair(pos.fX + 1, pos.fZ);
        } else if (type == "right" && facing == "west") {
            pair = make_pair(pos.fX, pos.fZ + 1);
        } else if (type == "left" && facing == "east") {
            pair = make_pair(pos.fX, pos.fZ + 1);
        }

        auto tag = std::make_shared<CompoundTag>();
        auto items = GetItems(*comp, "Items");

        tag->fValue = {
            {"Items", items},
            {"Findable", Bool(false)},
            {"id", String(string("Chest"))},
            {"isMovable", Bool(true)},
        };
        if (pair) {
            tag->fValue.emplace("pairlead", Bool(true));
            tag->fValue.emplace("pairx", Int(pair->first));
            tag->fValue.emplace("pairz", Int(pair->second));
        }
        Attach(pos, *tag);
        return tag;
    }
};

}
