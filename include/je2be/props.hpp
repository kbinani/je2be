#pragma once

namespace j2b {
namespace props {

inline std::shared_ptr<mcfile::nbt::ByteTag> Bool(bool b) {
    return std::make_shared<mcfile::nbt::ByteTag>(b ? 1 : 0);
}

inline std::shared_ptr<mcfile::nbt::ByteTag> Byte(int8_t v) {
    uint8_t t = *(uint8_t *)&v;
    return std::make_shared<mcfile::nbt::ByteTag>(t);
}

inline std::shared_ptr<mcfile::nbt::IntTag> Int(int32_t v) {
    return std::make_shared<mcfile::nbt::IntTag>(v);
}

inline std::shared_ptr<mcfile::nbt::LongTag> Long(int64_t v) {
    return std::make_shared<mcfile::nbt::LongTag>(v);
}

inline std::shared_ptr<mcfile::nbt::StringTag> String(std::string v) {
    return std::make_shared<mcfile::nbt::StringTag>(v);
}

inline std::shared_ptr<mcfile::nbt::FloatTag> Float(float v) {
    return std::make_shared<mcfile::nbt::FloatTag>(v);
}

}
}
