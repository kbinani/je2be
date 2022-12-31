#include <je2be/db/readonly-env.hpp>

#include <map>
#include <set>

using namespace std;
using namespace leveldb;
namespace fs = std::filesystem;

namespace je2be {

class ReadonlyEnv::Impl {
  struct VirtualFile {
    string fContents;
  };

  class WF : public WritableFile {
  public:
    WF(Impl *owner, fs::path p, string init) : fOwner(owner), fPath(p), fStorage(init) {}

    ~WF() {
      try {
        close();
      } catch (...) {
      }
    }

    Status Append(Slice const &data) override {
      fStorage += data.ToString();
      return Status::OK();
    }

    Status Close() override {
      return close();
    }

    Status Flush() override {
      return Status::OK();
    }

    Status Sync() override {
      return Status::OK();
    }

  private:
    Status close() {
      return fOwner->finalizeFile(fPath, fStorage);
    }

  private:
    Impl *const fOwner;
    fs::path const fPath;
    string fStorage;
  };

public:
  Impl() {}

  Status newWritableFile(fs::path const &f, WritableFile **r) {
    lock_guard<mutex> lock(fMut);
    if (fLocked.find(f) != fLocked.end()) {
      return Status::IOError("file locked");
    }
    auto wf = new WF(this, f, "");
    *r = wf;
    unsafeCreateNewFile(f);
    return Status::OK();
  }

  shared_ptr<VirtualFile> createNewFile(fs::path p) {
    lock_guard<mutex> lock(fMut);
    return unsafeCreateNewFile(p);
  }

  shared_ptr<VirtualFile> unsafeCreateNewFile(fs::path p) {
    auto vf = make_shared<VirtualFile>();
    fVirtualFiles[p] = vf;
    return vf;
  }

  Status finalizeFile(fs::path p, string contents) {
    lock_guard<mutex> lock(fMut);
    if (auto found = fVirtualFiles.find(p); found != fVirtualFiles.end()) {
      found->second->fContents = contents;
      return Status::OK();
    } else {
      return Status::IOError("");
    }
  }

private:
  mutex fMut;
  map<fs::path, shared_ptr<VirtualFile>> fVirtualFiles;
  set<fs::path> fLocked;
};

ReadonlyEnv::ReadonlyEnv() : EnvWrapper(Env::Default()), fImpl(new Impl()) {}

ReadonlyEnv::~ReadonlyEnv() {}

Status ReadonlyEnv::NewWritableFile(fs::path const &f, WritableFile **r) {
  return fImpl->newWritableFile(f, r);
}

Status ReadonlyEnv::NewAppendableFile(fs::path const &f, WritableFile **r) {
  return Status::NotSupported("");
}

Status ReadonlyEnv::RemoveFile(fs::path const &f) {
  return Status::NotSupported("");
}

Status ReadonlyEnv::RemoveDir(fs::path const &d) {
  return Status::NotSupported("");
}

Status ReadonlyEnv::RenameFile(fs::path const &s, fs::path const &t) {
  return Status::NotSupported("");
}

Status ReadonlyEnv::LockFile(fs::path const &f, FileLock **l) {
  return Status::NotSupported("");
}

Status ReadonlyEnv::UnlockFile(FileLock *l) {
  return Status::NotSupported("");
}

} // namespace je2be
