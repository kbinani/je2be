#pragma once

#include "db/_db-interface.hpp"

#include <filesystem>
#include <memory>

namespace je2be {

class RawDb : public DbInterface {
public:
  RawDb(std::filesystem::path const &dir);

  RawDb(RawDb &&) = delete;
  RawDb &operator=(RawDb &&) = delete;

  ~RawDb();

  bool valid() const override;
  void put(std::string const &key, leveldb::Slice const &value) override;
  void del(std::string const &key) override {
    // nop because write-only
  }
  bool close(std::function<void(double progress)> progress = nullptr) override;
  void abandon() override;

private:
  class Impl;
  std::unique_ptr<Impl> fImpl;
};

} // namespace je2be