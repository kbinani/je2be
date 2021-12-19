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
#include <db/version_edit.h>
#include <db/log_writer.h>
#include <xxhash32.h>
#include <xxhash64.h>
#include <hwm/task/task_queue.hpp>
#include <nlohmann/json.hpp>

#include <set>
#include <unordered_set>
#include <execution>
#include <random>

#include <je2be/file.hpp>
#include <je2be/scoped-file.hpp>
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
#include <je2be/size.hpp>
#include <je2be/xxhash.hpp>
#include <je2be/uuid.hpp>
#include <je2be/props.hpp>
#include <je2be/progress.hpp>
#include <je2be/version.hpp>
#include <je2be/future-support.hpp>
#include <je2be/fs.hpp>

#include <je2be/tobe/uuid-registrar.hpp>
#include <je2be/tobe/versions.hpp>
#include <je2be/tobe/sign.hpp>
#include <je2be/tobe/block-data.hpp>
#include <je2be/tobe/biome-map-legacy.hpp>
#include <je2be/tobe/db/db-interface.hpp>
#include <je2be/tobe/db/db.hpp>
#include <je2be/tobe/db/null-db.hpp>
#include <je2be/tobe/db/raw-db.hpp>
#include <je2be/tobe/player-abilities.hpp>
#include <je2be/tobe/level-data.hpp>
#include <je2be/tobe/height-map.hpp>
#include <je2be/tobe/options.hpp>
#include <je2be/tobe/java-edition-map.hpp>
#include <je2be/tobe/portal/portal.hpp>
#include <je2be/tobe/portal/oriented-portal-blocks.hpp>
#include <je2be/tobe/portal/portal-blocks.hpp>
#include <je2be/tobe/portal/portals.hpp>
#include <je2be/tobe/map.hpp>
#include <je2be/tobe/structure/structure-piece.hpp>
#include <je2be/tobe/structure/structure-piece-collection.hpp>
#include <je2be/tobe/structure/structures.hpp>
#include <je2be/tobe/statistics.hpp>
#include <je2be/tobe/world-data.hpp>
#include <je2be/tobe/dimension-data-fragment.hpp>
#include <je2be/tobe/potion-data.hpp>
#include <je2be/tobe/enchant-data.hpp>
#include <je2be/tobe/fireworks-explosion.hpp>
#include <je2be/tobe/fireworks.hpp>
#include <je2be/tobe/entity-attributes.hpp>
#include <je2be/tobe/tropical-fish.hpp>
#include <je2be/tobe/axolotl.hpp>
#include <je2be/tobe/item.hpp>
#include <je2be/tobe/entity.hpp>
#include <je2be/tobe/tile-entity.hpp>
#include <je2be/tobe/chunk-conversion-mode.hpp>
#include <je2be/tobe/chunk-data.hpp>
#include <je2be/tobe/chunk-data-package.hpp>
#include <je2be/tobe/block-palette.hpp>
#include <je2be/tobe/moving-piston.hpp>
#include <je2be/tobe/datapacks.hpp>
#include <je2be/tobe/converter.hpp>
