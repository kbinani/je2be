#pragma once

namespace j2b {

class DbInterface {
public:
  virtual ~DbInterface() {}
  virtual bool valid() const = 0;
  virtual void put(std::string const &key, leveldb::Slice const &value) = 0;
  virtual void del(std::string const &key) = 0;
};

} // namespace j2b
