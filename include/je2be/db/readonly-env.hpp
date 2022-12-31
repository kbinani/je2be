#pragma once

#include <filesystem>
#include <memory>

#include <leveldb/env.h>

namespace je2be {

class ReadonlyEnv : public leveldb::EnvWrapper {
  class Impl;

public:
  ReadonlyEnv();
  virtual ~ReadonlyEnv();

  leveldb::Status NewWritableFile(const std::filesystem::path &f, leveldb::WritableFile **r) override;
  leveldb::Status NewAppendableFile(const std::filesystem::path &f, leveldb::WritableFile **r) override;
  leveldb::Status RemoveFile(const std::filesystem::path &f) override;
  leveldb::Status RemoveDir(const std::filesystem::path &d) override;
  leveldb::Status RenameFile(const std::filesystem::path &s, const std::filesystem::path &t) override;
  leveldb::Status LockFile(const std::filesystem::path &f, leveldb::FileLock **l) override;
  leveldb::Status UnlockFile(leveldb::FileLock *l) override;

private:
  std::unique_ptr<Impl> fImpl;
};

} // namespace je2be
