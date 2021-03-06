#pragma once

namespace j2b {

class Db : public DbInterface {
public:
  explicit Db(std::string const &dir) : fDb(nullptr), fValid(false) {
    using namespace leveldb;

    DB *db;
    Options options;
    options.compression = kZlibRawCompression;
    options.create_if_missing = true;
    Status status = DB::Open(options, dir.c_str(), &db);
    if (!status.ok()) {
      return;
    }
    fDb = db;
    fValid = true;
  }

  ~Db() {
    if (fValid) {
      delete fDb;
    }
  }

  bool valid() const override { return fValid; }

  void put(std::string const &key, leveldb::Slice const &value) override {
    assert(fValid);
    fDb->Put(fWriteOptions, key, value);
  }

  void write(leveldb::WriteBatch &batch) {
    assert(fValid);
    fDb->Write(leveldb::WriteOptions{}, &batch);
  }

  std::optional<std::string> get(std::string const &key) {
    assert(fValid);
    leveldb::ReadOptions o;
    std::string v;
    leveldb::Status st = fDb->Get(o, key, &v);
    if (st.ok()) {
      return v;
    } else {
      return std::nullopt;
    }
  }

  void del(std::string const &key) override {
    assert(fValid);
    fDb->Delete(leveldb::WriteOptions{}, key);
  }

private:
  leveldb::DB *fDb;
  bool fValid;
  leveldb::WriteOptions fWriteOptions;
};

} // namespace j2b
