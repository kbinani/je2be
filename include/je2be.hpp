#pragma once

// clang-format off

#include <minecraft-file.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/comparator.h>
#include <leveldb/table_builder.h>
#include <leveldb/env.h>
#include <leveldb/table.h>
#include <table/compression/zlib_compressor.h>
#include <db/dbformat.h>
#include <xxhash.h>
#include <ThreadPool.h>
#include <nlohmann/json.hpp>

#include <set>
#include <unordered_set>

#include <je2be/algorithm.hpp>
#include <je2be/file.hpp>
#include <je2be/strings.hpp>
#include <je2be/enums/banner-color-code-bedrock.hpp>
#include <je2be/enums/color-code-java.hpp>
#include <je2be/enums/level-directory-structure.hpp>
#include <je2be/enums/villager-profession.hpp>
#include <je2be/enums/villager-type.hpp>
#include <je2be/pos2.hpp>
#include <je2be/pos3.hpp>
#include <je2be/volume.hpp>
#include <je2be/optional.hpp>
#include <je2be/vec.hpp>
#include <je2be/rotation.hpp>
#include <je2be/color/rgba.hpp>
#include <je2be/color/lab.hpp>
#include <je2be/sign.hpp>
#include <je2be/size.hpp>
#include <je2be/xxhash.hpp>
#include <je2be/props.hpp>
#include <je2be/block-data.hpp>
#include <je2be/biome-map.hpp>
#include <je2be/version.hpp>
#include <je2be/db/db-interface.hpp>
#include <je2be/db/db.hpp>
#include <je2be/db/null-db.hpp>
#include <je2be/db/async-db.hpp>
#include <je2be/db/raw-db.hpp>
#include <je2be/level-data.hpp>
#include <je2be/height-map.hpp>
#include <je2be/options.hpp>
#include <je2be/java-edition-map.hpp>
#include <je2be/portal/portal.hpp>
#include <je2be/portal/oriented-portal-blocks.hpp>
#include <je2be/portal/portal-blocks.hpp>
#include <je2be/portal/portals.hpp>
#include <je2be/map.hpp>
#include <je2be/structure/structure-piece.hpp>
#include <je2be/structure/structure-piece-collection.hpp>
#include <je2be/structure/structures.hpp>
#include <je2be/statistics.hpp>
#include <je2be/world-data.hpp>
#include <je2be/dimension-data-fragment.hpp>
#include <je2be/potion-data.hpp>
#include <je2be/enchant-data.hpp>
#include <je2be/fireworks-explosion.hpp>
#include <je2be/fireworks.hpp>
#include <je2be/entity-attributes.hpp>
#include <je2be/tropical-fish.hpp>
#include <je2be/axolotl.hpp>
#include <je2be/item.hpp>
#include <je2be/entity.hpp>
#include <je2be/tile-entity.hpp>
#include <je2be/chunk-data.hpp>
#include <je2be/chunk-data-package.hpp>
#include <je2be/progress.hpp>
#include <je2be/block-palette.hpp>
#include <je2be/moving-piston.hpp>
#include <je2be/converter.hpp>
