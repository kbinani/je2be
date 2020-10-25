#pragma once

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#endif

#include <minecraft-file.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <table/compression/zlib_compressor.h>
#include <xxhash.h>
#include <ThreadPool.h>

#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <je2be/file.hpp>
#include <je2be/enums.hpp>
#include <je2be/pos.hpp>
#include <je2be/key.hpp>
#include <je2be/props.hpp>
#include <je2be/block-data.hpp>
#include <je2be/biome-map.hpp>
#include <je2be/version.hpp>
#include <je2be/db/db-interface.hpp>
#include <je2be/db/db.hpp>
#include <je2be/db/deferred-db.hpp>
#include <je2be/db/null-db.hpp>
#include <je2be/level-data.hpp>
#include <je2be/height-map.hpp>
#include <je2be/entity.hpp>
#include <je2be/tile-entity.hpp>
#include <je2be/entities/entities.hpp>
#include <je2be/chunk-data.hpp>
#include <je2be/portal.hpp>
#include <je2be/oriented-portal-blocks.hpp>
#include <je2be/portal-blocks.hpp>
#include <je2be/portals.hpp>
#include <je2be/chunk-data-package.hpp>
#include <je2be/world-data-package.hpp>
#include <je2be/converter.hpp>