#pragma once

namespace j2b {

class Entity {
private:
    using CompoundTag = mcfile::nbt::CompoundTag;
    using EntityDataType = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using Converter = std::function<EntityDataType(CompoundTag const&)>;

public:
    explicit Entity(int64_t uid) : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

    static std::shared_ptr<mcfile::nbt::CompoundTag> From(mcfile::nbt::CompoundTag const& tag) {
        using namespace props;
        auto id = GetString(tag, "id");
        if (!id) return nullptr;
        static std::unique_ptr<std::unordered_map<std::string, Converter> const> const table(CreateEntityTable());
        auto found = table->find(*id);
        if (found == table->end()) {
            return Default(tag);
        }
        return found->second(tag);
    }

    static bool DegAlmostEquals(float a, float b) {
        a = fmod(fmod(a, 360) + 360, 360);
        assert(0 <= a && a < 360);
        b = fmod(fmod(b, 360) + 360, 360);
        assert(0 <= b && b < 360);
        float diff = fmod(fmod(a - b, 360) + 360, 360);
        assert(0 <= diff && diff < 360);
        float const tolerance = 1;
        if (0 <= diff && diff < tolerance) {
            return true;
        } else if (360 - tolerance < diff) {
            return true;
        } else {
            return false;
        }
    }

    static bool RotAlmostEquals(Rotation const& rot, float yaw, float pitch) {
        return DegAlmostEquals(rot.fYaw, yaw) && DegAlmostEquals(rot.fPitch, pitch);
    }

    static std::tuple<Pos, std::shared_ptr<CompoundTag>, std::string> ToTileEntityBlock(CompoundTag const& c) {
        using namespace std;
        using namespace props;
        auto id = GetString(c, "id");
        assert(id);
        if (*id == "minecraft:item_frame") {
            auto tileX = GetInt(c, "TileX");
            auto tileY = GetInt(c, "TileY");
            auto tileZ = GetInt(c, "TileZ");

            auto rot = GetRotation(c, "Rotation");
            int32_t facing = 0;
            if (RotAlmostEquals(*rot, 0, -90)) {
                // up
                facing = 1;
            } else if (RotAlmostEquals(*rot, 180, 0)) {
                // north
                facing = 2;
            } else if (RotAlmostEquals(*rot, 270, 0)) {
                // east
                facing = 5;
            } else if (RotAlmostEquals(*rot, 0, 0)) {
                // south
                facing = 3;
            } else if (RotAlmostEquals(*rot, 90, 0)) {
                // west
                facing = 4;
            } else if (RotAlmostEquals(*rot, 0, 90)) {
                // down
                facing = 0;
            }

            bool map = false;
            auto itemId = c.query("Item/id");
            if (itemId) {
                auto itemIdString = itemId->asString();
                if (itemIdString) {
                    string itemName = itemIdString->fValue;
                    if (itemName == "minecraft:filled_map") {
                        map = true;
                    }
                }
            }

            auto b = make_shared<CompoundTag>();
            auto states = make_shared<CompoundTag>();
            states->fValue = {
                {"facing_direction", Int(facing)},
                {"item_frame_map_bit", Bool(map)},
            };
            string key = "minecraft:frame[facing_direction=" + to_string(facing) + ",item_frame_map_bit=" + (map ? "true" : "false") + "]";
            b->fValue = {
                {"name", String("minecraft:frame")},
                {"version", Int(BlockData::kBlockDataVersion)},
                {"states", states},
            };
            Pos pos(*tileX, *tileY, *tileZ);
            return make_tuple(pos, b, key);
        }
    }

    static TileEntity::TileEntityData ToTileEntityData(CompoundTag const& c, JavaEditionMap const& mapInfo, DimensionDataFragment &ddf) {
        using namespace props;
        auto id = GetString(c, "id");
        assert(id);
        if (*id == "minecraft:item_frame") {
            auto tag = std::make_shared<CompoundTag>();
            auto tileX = GetInt(c, "TileX");
            auto tileY = GetInt(c, "TileY");
            auto tileZ = GetInt(c, "TileZ");
            if (!tileX || !tileY || !tileZ) return nullptr;
            tag->fValue = {
                {"id", String("ItemFrame")},
                {"isMovable", Bool(true)},
                {"x", Int(*tileX)},
                {"y", Int(*tileY)},
                {"z", Int(*tileZ)},
            };
            auto itemRotation = GetByteOrDefault(c, "ItemRotation", 0);
            auto itemDropChange = GetFloatOrDefault(c, "ItemDropChange", 1);
            auto found = c.fValue.find("Item");
            if (found != c.fValue.end() && found->second->id() == mcfile::nbt::Tag::TAG_Compound) {
                auto item = std::dynamic_pointer_cast<CompoundTag>(found->second);
                auto m = Item::From(item, mapInfo, ddf);
                if (m) {
                    tag->fValue.insert(make_pair("Item", m));
                    tag->fValue.insert(make_pair("ItemRotation", Float(itemRotation)));
                    tag->fValue.insert(make_pair("ItemDropChange", Float(itemDropChange)));
                }
            }
            return tag;
        }
    }

    static bool IsTileEntity(CompoundTag const& tag) {
        auto id = props::GetString(tag, "id");
        if (!id) return false;
        return *id == "minecraft:item_frame";
    }

    std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;
        auto tag = make_shared<CompoundTag>();
        auto tags = make_shared<ListTag>();
        tags->fType = Tag::TAG_Compound;
        auto definitions = make_shared<ListTag>();
        definitions->fType = Tag::TAG_String;
        for (auto const& d : fDefinitions) {
            definitions->fValue.push_back(String(d));
        }
        tag->fValue = {
            {"definitions", definitions},
            {"Motion", fMotion.toListTag()},
            {"Pos", fPos.toListTag()},
            {"Rotation", fRotation.toListTag()},
            {"Tags", tags},
            {"Chested", Bool(fChested)},
            {"Color2", Byte(fColor2)},
            {"Color", Byte(fColor)},
            {"Dir", Byte(fDir)},
            {"FallDistance", Float(fFallDistance)},
            {"Fire", Short(std::max((int16_t)0, fFire))},
            {"identifier", String(fIdentifier)},
            {"Invulnerable", Bool(fInvulnerable)},
            {"IsAngry", Bool(fIsAngry)},
            {"IsAutonomous", Bool(fIsAutonomous)},
            {"IsBaby", Bool(fIsBaby)},
            {"IsEating", Bool(fIsEating)},
            {"IsGliding", Bool(fIsGliding)},
            {"IsGlobal", Bool(fIsGlobal)},
            {"IsIllagerCaptain", Bool(fIsIllagerCaptain)},
            {"IsOrphaned", Bool(fIsOrphaned)},
            {"IsRoaring", Bool(fIsRoaring)},
            {"IsScared", Bool(fIsScared)},
            {"IsStunned", Bool(fIsStunned)},
            {"IsSwimming", Bool(fIsSwimming)},
            {"IsTamed", Bool(fIsTamed)},
            {"IsTrusting", Bool(fIsTrusting)},
            {"LastDimensionId", Int(fLastDimensionId)},
            {"LootDropped", Bool(fLootDropped)},
            {"MarkVariant", Int(fMarkVariant)},
            {"OnGround", Bool(fOnGround)},
            {"OwnerNew", Int(fOwnerNew)},
            {"PortalCooldown", Int(fPortalCooldown)},
            {"Saddled", Bool(fSaddled)},
            {"Sheared", Bool(fSheared)},
            {"ShowBottom", Bool(fShowBottom)},
            {"Sitting", Bool(fSitting)},
            {"SkinID", Int(fSkinId)},
            {"Strength", Int(fStrength)},
            {"StrengthMax", Int(fStrengthMax)},
            {"UniqueID", Long(fUniqueId)},
            {"Variant", Int(fVariant)},
        };
        if (fCustomName) {
            tag->fValue["CustomName"] = String(*fCustomName);
            tag->fValue["CustomNameVisible"] = Bool(fCustomNameVisible);
        }
        return tag;
    }

private:
    static std::unordered_map<std::string, Converter>* CreateEntityTable() {
        auto table = new std::unordered_map<std::string, Converter>();
#define E(__name, __func) table->insert(std::make_pair("minecraft:" __name, __func))
        E("painting", Painting);
        E("end_crystal", EndCrystal);
        E("chicken", Animal);
        E("cow", Animal);
        E("donkey", Animal);
        E("cat", LivingEntity(Animal, Ageable("cat"), Tameable("cat"), Sittable, Cat));
        E("dolphin", Animal);
        E("cod", Animal);
        E("bee", Animal);
        E("bat", Bat);
#undef E
        return table;
    }

    static EntityDataType Bat(CompoundTag const& tag) {
        using namespace mcfile::nbt;
        auto ret = Animal(tag);
        auto& c = *ret;
        auto batFlags = props::GetBoolOrDefault(tag, "BatFlags", false);
        c["BatFlags"] = props::Bool(batFlags);
        AppendDefinition(ret, "+minecraft:bat");
        return ret;
    }

    using Behavior = std::function<EntityDataType(EntityDataType const&, CompoundTag const& input)>;

    struct LivingEntity {
        template <class ...Arg>
        LivingEntity(Converter base, Arg ...args) : fBase(base), fBehaviors(std::initializer_list<Behavior>{args...}) {
        }

        EntityDataType operator()(CompoundTag const& input) const {
            auto c = fBase(input);
            if (!c) return nullptr;
            for (auto const& b : fBehaviors) {
                auto next = b(c, input);
                if (!next) continue;
                c = next;
            }
            return c;
        }

    private:
        Converter fBase;
        std::vector<Behavior> fBehaviors;
    };

    static EntityDataType Cat(EntityDataType const& c, CompoundTag const& tag) {
        using namespace mcfile::nbt;
        using namespace std;
        auto collarColor = props::GetByte(tag, "CollarColor");
        if (collarColor && props::GetUUID(tag, "Owner")) {
            c->fValue["Color"] = props::Byte(*collarColor);
        }
        auto catType = props::GetInt(tag, "CatType");
        if (catType) {
            int32_t variant = 0;
            std::string type;
            switch (*catType) {
            case 0:
                type = "tabby";
                variant = 8;
                break;
            case 1:
                type = "tuxedo";
                variant = 1;
                break;
            case 2:
                type = "red";
                variant = 2;
                break;
            case 3:
                type = "siamese";
                variant = 3;
                break;
            case 4:
                type = "british";
                variant = 4;
                break;
            case 5:
                type = "calico";
                variant = 5;
                break;
            case 6:
                type = "persian";
                variant = 6;
                break;
            case 7:
                type = "ragdoll";
                variant = 7;
                break;
            case 8:
                type = "white";
                variant = 0;
                break;
            case 9:
                type = "jellie";
                variant = 10;
                break;
            case 10:
                type = "black";
                variant = 9;
                break;
            }
            if (!type.empty()) {
                AppendDefinition(c, "+minecraft:cat_" + type);
            }
            c->fValue["Variant"] = props::Int(variant);
        }
        c->fValue["DwellingUniqueID"] = props::String("00000000-0000-0000-0000-000000000000");
        c->fValue["RewardPlayersOnFirstFounding"] = props::Bool(true);
        auto attributes = std::make_shared<ListTag>();
        attributes->fType = Tag::TAG_Compound;
        attributes->fValue = {
            Attribute(0, 0, 1024, "minecraft:luck"),
            Attribute(10, 10, 10, "minecraft:health"),
            Attribute(0, 0, 16, "minecraft:absorption"),
            Attribute(0, 0, 1, "minecraft:knockback_resistance"),
            Attribute(0.3f, 0.3f, std::numeric_limits<float>::max(), "minecraft:movement"),
            Attribute(0.02f, 0.02f, std::numeric_limits<float>::max(), "minecraft:underwater_movement"),
            Attribute(0.02f, 0.02f, std::numeric_limits<float>::max(), "minecraft:lava_movement"),
            Attribute(16, 16, 2048, "minecraft:follow_range"),
            Attribute(4, 4, std::numeric_limits<float>::max(), "minecraft:attack_damage"),
        };
        c->fValue["Attributes"] = attributes;
        return c;
    }

    static std::shared_ptr<CompoundTag> Attribute(float base, float current, float max, std::string const& name) {
        auto a = std::make_shared<CompoundTag>();
        a->fValue["Base"] = props::Float(base);
        a->fValue["Current"] = props::Float(current);
        a->fValue["Max"] = props::Float(max);
        a->fValue["Name"] = props::String(name);
        return a;
    }

    static Behavior Ageable(std::string const& definitionKey) {
        return [=](EntityDataType const& c, CompoundTag const& tag) {
            auto age = props::GetIntOrDefault(tag, "Age", 0);
            if (age < 0) {
                AppendDefinition(c, "+minecraft:" + definitionKey + "_baby");
                c->fValue["Age"] = props::Int(age);
            } else {
                AppendDefinition(c, "+minecraft:" + definitionKey + "_adult");
                c->fValue.erase("Age");
            }
            c->fValue["IsBaby"] = props::Bool(age < 0);
            return c;
        };
    }

    static Behavior Tameable(std::string const& definitionKey) {
        return [=](EntityDataType const& c, CompoundTag const& tag) {
            auto owner = props::GetUUID(tag, "Owner");
            if (owner) {
                c->fValue["OwnerNew"] = props::Long(*owner);
                AppendDefinition(c, "+minecraft:" + definitionKey + "_tame");
                c->fValue["IsTamed"] = props::Bool(true);
            }
            AppendDefinition(c, "+minecraft:" + definitionKey + "_wild");
            return c;
        };
    }

    static void AppendDefinition(EntityDataType const& c, std::string const& definition) {
        using namespace mcfile::nbt;

        auto found = c->fValue.find("definitions");
        auto d = std::make_shared<ListTag>();
        d->fType = Tag::TAG_String;
        if (found != c->fValue.end()) {
            auto current = found->second->asList();
            if (current && current->fType == Tag::TAG_String) {
                for (auto c : current->fValue) {
                    if (c->asString()) {
                        d->fValue.push_back(props::String(c->asString()->fValue));
                    }
                }
            }
        }
        d->fValue.push_back(props::String(definition));
        c->fValue["definitions"] = d;
    }

    static EntityDataType Sittable(EntityDataType const& c, CompoundTag const& tag) {
        auto sitting = props::GetBoolOrDefault(tag, "Sitting", false);
        c->fValue["Sitting"] = props::Bool(sitting);
        return c;
    }

    static EntityDataType Animal(CompoundTag const& tag) {
        using namespace props;
        auto e = BaseProperties(tag);
        if (!e) return nullptr;
        auto ret = e->toCompoundTag();
        auto& c = *ret;
        auto air = GetShortOrDefault(tag, "Air", 300);
        auto armor = GetArmor(tag);
        auto mainhand = GetMainhand(tag);
        auto offhand = GetOffhand(tag);
        auto canPickupLoot = GetBoolOrDefault(tag, "CanPickUpLoot", false);
        auto deathTime = GetShortOrDefault(tag, "DeathTime", 0);
        auto hurtTime = GetShortOrDefault(tag, "HurtTime", 0);
        auto inLove = GetBoolOrDefault(tag, "InLove", false);
        c["Armor"] = armor;
        c["Mainhand"] = mainhand;
        c["Offhand"] = offhand;
        c["Air"] = Short(air);
        c["AttackTime"] = Short(0);
        c["BodyRot"] = Float(0);
        c["boundX"] = Int(0);
        c["boundY"] = Int(0);
        c["boundZ"] = Int(0);
        c["BreedCooldown"] = Int(0);
        c["canPickupItems"] = Bool(canPickupLoot);
        c["Dead"] = Bool(false);
        c["DeathTime"] = Short(0);
        c["hasBoundOrigin"] = Bool(false);
        c["hasSetCanPickupItems"] = Bool(true);
        c["HurtTime"] = Short(0);
        c["InLove"] = Bool(false);
        c["IsPregnant"] = Bool(false);
        c["LeasherID"] = Long(-1);
        c["limitedLife"] = Int(0);
        c["LoveCause"] = Long(0);
        c["NaturalSpawn"] = Bool(false);
        c["Persistent"] = Bool(true);
        c["Surface"] = Bool(false);
        c["TargetID"] = Long(-1);
        c["TradeExperience"] = Int(0);
        c["TradeTier"] = Int(0);
        AppendDefinition(ret, "+" + e->fIdentifier);
        ret->fValue.erase("Motion");
        ret->fValue.erase("Dir");
        return ret;
    }

    static std::shared_ptr<mcfile::nbt::ListTag> GetArmor(CompoundTag const& tag) {
        using namespace mcfile::nbt;
        auto ret = std::make_shared<ListTag>();
        ret->fType = Tag::TAG_Compound;

        for (int i = 0; i < 4; i++) {
            auto item = tag.query("ArmorItems/" + std::to_string(i))->asCompound();
            std::shared_ptr<CompoundTag> armor;
            if (item) {
                //TODO: 
            }
            if (!armor) {
                armor = Item::Empty();
            }
            ret->fValue.push_back(armor);
        }
        return ret;
    }

    static std::shared_ptr<mcfile::nbt::ListTag> GetMainhand(CompoundTag const& tag) {
        using namespace mcfile::nbt;
        auto ret = std::make_shared<ListTag>();
        ret->fType = Tag::TAG_Compound;

        auto armor = Item::Empty(); //TODO:
        ret->fValue.push_back(armor);

        return ret;
    }

    static std::shared_ptr<mcfile::nbt::ListTag> GetOffhand(CompoundTag const& tag) {
        using namespace mcfile::nbt;
        auto ret = std::make_shared<ListTag>();
        ret->fType = Tag::TAG_Compound;

        auto armor = Item::Empty(); //TODO:
        ret->fValue.push_back(armor);

        return ret;
    }

    static std::optional<Entity> BaseProperties(CompoundTag const& tag) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto fallDistance = GetFloat(tag, "FallDistance");
        auto fire = GetShort(tag, "Fire");
        auto invulnerable = GetBool(tag, "Invulnerable");
        auto onGround = GetBool(tag, "OnGround");
        auto portalCooldown = GetInt(tag, "PortalCooldown");
        auto motion = GetVec(tag, "Motion");
        auto pos = GetVec(tag, "Pos");
        auto rotation = GetRotation(tag, "Rotation");
        auto uuid = GetUUID(tag, "UUID");
        auto id = GetString(tag, "id");
        auto customName = GetJson(tag, "CustomName");

        if (!uuid) return nullopt;

        Entity e(*uuid);
        if (motion) e.fMotion = *motion;
        if (pos) e.fPos = *pos;
        if (rotation) e.fRotation = *rotation;
        if (fallDistance) e.fFallDistance = *fallDistance;
        if (fire) e.fFire = *fire;
        if (invulnerable) e.fInvulnerable = *invulnerable;
        if (onGround) e.fOnGround = *onGround;
        if (portalCooldown) e.fPortalCooldown = *portalCooldown;
        if (id) e.fIdentifier = *id;
        if (customName && (*customName)["text"].is_string()) {
            auto text = (*customName)["text"].get<std::string>();
            e.fCustomName = text;
            e.fCustomNameVisible = true;
        }

        return e;
    }

    static EntityDataType Default(CompoundTag const& tag) {
        auto e = BaseProperties(tag);
        if (!e) {
            return nullptr;
        }
        return e->toCompoundTag();
    }

    static EntityDataType EndCrystal(CompoundTag const& tag) {
        auto e = BaseProperties(tag);
        if (!e) return nullptr;
        e->fIdentifier = "minecraft:ender_crystal";
        auto c = e->toCompoundTag();
        return c;
    }

    static EntityDataType Painting(CompoundTag const& tag) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto facing = GetByte(tag, "Facing");
        auto motive = GetString(tag, "Motive");
        auto beMotive = PaintingMotive(*motive);
        auto size = PaintingSize(*motive);
        if (!beMotive || !size) return nullptr;

        auto tileX = GetInt(tag, "TileX");
        auto tileY = GetInt(tag, "TileY");
        auto tileZ = GetInt(tag, "TileZ");
        if (!tileX || !tileY || !tileZ) return nullptr;

        Vec normals[4] = {Vec(0, 0, 1), Vec(-1, 0, 0), Vec(0, -1, 0), Vec(1, 0, 0)};
        Vec normal = normals[*facing];

        float const thickness = 1.0f / 32.0f;

        int dh = 0;
        int dv = 0;
        if (size->fWidth >= 4) {
            dh = 1;
        }
        if (size->fHeight >= 3) {
            dv = 1;
        }

        float x, z;
        float y = *tileY - dv + size->fHeight * 0.5f;

        // facing
        // 0: south
        // 1: west
        // 2: north
        // 3: east
        if (*facing == 0) {
            x = *tileX - dh + size->fWidth * 0.5f;;
            z = *tileZ + thickness;
        } else if (*facing == 1) {
            x = *tileX + 1 - thickness;
            z = *tileZ - dh + size->fWidth * 0.5f;
        } else if (*facing == 2) {
            x = *tileX + 1 + dh - size->fWidth * 0.5f;
            z = *tileZ + 1 - thickness;
        } else {
            x = *tileX + thickness;
            z = *tileZ + 1 + dh - size->fWidth * 0.5f;
        }

        auto e = BaseProperties(tag);
        if (!e) return nullptr;
        e->fIdentifier = "minecraft:painting";
        e->fPos = Vec(x, y, z);
        auto c = e->toCompoundTag();
        c->fValue.emplace("Motive", String(*beMotive));
        c->fValue.emplace("Direction", Byte(*facing));
        return c;
    }

    static std::optional<Size> PaintingSize(std::string const& motive) {
        using namespace std;
        static unordered_map<string, Size> const mapping = {
            {"minecraft:pigscene", Size(4, 4)},
            {"minecraft:burning_skull", Size(4, 4)},
            {"minecraft:pointer", Size(4, 4)},
            {"minecraft:skeleton", Size(4, 3)},
            {"minecraft:donkey_kong", Size(4, 3)},
            {"minecraft:fighters", Size(4, 2)},
            {"minecraft:skull_and_roses", Size(2, 2)},
            {"minecraft:match", Size(2, 2)},
            {"minecraft:bust", Size(2, 2)},
            {"minecraft:stage", Size(2, 2)},
            {"minecraft:void", Size(2, 2)},
            {"minecraft:wither", Size(2, 2)},
            {"minecraft:sunset", Size(2, 1)},
            {"minecraft:courbet", Size(2, 1)},
            {"minecraft:creebet", Size(2, 1)},
            {"minecraft:sea", Size(2, 1)},
            {"minecraft:wanderer", Size(1, 2)},
            {"minecraft:graham", Size(1, 2)},
            {"minecraft:aztec2", Size(1, 1)},
            {"minecraft:alban", Size(1, 1)},
            {"minecraft:bomb", Size(1, 1)},
            {"minecraft:kebab", Size(1, 1)},
            {"minecraft:wasteland", Size(1, 1)},
            {"minecraft:aztec", Size(1, 1)},
            {"minecraft:plant", Size(1, 1)},
            {"minecraft:pool", Size(2, 1)},
        };
        auto found = mapping.find(motive);
        if (found != mapping.end()) {
            return found->second;
        }
        return nullopt;
    }

    static std::optional<std::string> PaintingMotive(std::string const& je) {
        using namespace std;
        static unordered_map<string, string> const mapping = {
            {"minecraft:bust", "Bust"},
            {"minecraft:pigscene", "Pigscene"},
            {"minecraft:burning_skull", "BurningSkull"},
            {"minecraft:pointer", "Pointer"},
            {"minecraft:skeleton", "Skeleton"},
            {"minecraft:donkey_kong", "DonkeyKong"},
            {"minecraft:fighters", "Fighters"},
            {"minecraft:skull_and_roses", "SkullAndRoses"},
            {"minecraft:match", "Match"},
            {"minecraft:bust", "Bust"},
            {"minecraft:stage", "Stage"},
            {"minecraft:void", "Void"},
            {"minecraft:wither", "Wither"},
            {"minecraft:sunset", "Sunset"},
            {"minecraft:courbet", "Courbet"},
            {"minecraft:creebet", "Creebet"},
            {"minecraft:sea", "Sea"},
            {"minecraft:wanderer", "Wanderer"},
            {"minecraft:graham", "Graham"},
            {"minecraft:aztec2", "Aztec2"},
            {"minecraft:alban", "Alban"},
            {"minecraft:bomb", "Bomb"},
            {"minecraft:kebab", "Kebab"},
            {"minecraft:wasteland", "Wasteland"},
            {"minecraft:aztec", "Aztec"},
            {"minecraft:plant", "Plant"},
            {"minecraft:pool", "Pool"},
        };
        auto found = mapping.find(je);
        if (found != mapping.end()) {
            return found->second;
        }
        return nullopt;
    }

public:
    std::vector<std::string> fDefinitions;
    Vec fMotion;
    Vec fPos;
    Rotation fRotation;
    std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fTags;
    bool fChested = false;
    int8_t fColor2 = 0;
    int8_t fColor = 0;
    int8_t fDir = 0;
    float fFallDistance = 0;
    int16_t fFire = 0;
    std::string fIdentifier;
    bool fInvulnerable = false;
    bool fIsAngry = false;
    bool fIsAutonomous = false;
    bool fIsBaby = false;
    bool fIsEating = false;
    bool fIsGliding = false;
    bool fIsGlobal = false;
    bool fIsIllagerCaptain = false;
    bool fIsOrphaned = false;
    bool fIsRoaring = false;
    bool fIsScared = false;
    bool fIsStunned = false;
    bool fIsSwimming = false;
    bool fIsTamed = false;
    bool fIsTrusting = false;
    int32_t fLastDimensionId = 0;
    bool fLootDropped = false;
    int32_t fMarkVariant = 0;
    bool fOnGround = true;
    int64_t fOwnerNew = -1;
    int32_t fPortalCooldown = 0;
    bool fSaddled = false;
    bool fSheared = false;
    bool fShowBottom = false;
    bool fSitting = false;
    int32_t fSkinId = 0;
    int32_t fStrength = 0;
    int32_t fStrengthMax = 0;
    int64_t const fUniqueId;
    int32_t fVariant = 0;
    std::optional<std::string> fCustomName;
    bool fCustomNameVisible = false;
};

}
