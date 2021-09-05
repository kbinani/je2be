#pragma once

namespace je2be::tobe {

class RawDb : public DbInterface {
public:
  RawDb(std::string const &, unsigned int concurrency) = delete;
  RawDb(std::wstring const &, unsigned int concurrency) = delete;
  RawDb(std::filesystem::path const &dir, unsigned int concurrency) : fValid(true), fDir(dir) {
    using namespace std;
    using namespace leveldb;
    namespace fs = std::filesystem;

    leveldb::FileLock *lf = nullptr;
    auto lockFileName = dir / "LOCK";
    auto env = leveldb::Env::Default();
    Status st = env->LockFile(lockFileName, &lf);
    if (!st.ok()) {
      fValid = false;
      return;
    }
    fFileLock = lf;

    fStop.store(false);
    fAbandoned.store(false);

    thread th([this, concurrency]() {
      for (auto const &e : fs::directory_iterator(fs::path(fDir))) {
        if (e.path().filename() != "LOCK") {
          fs::remove_all(e.path());
        }
      }

      Options o;
      o.compression = kZlibRawCompression;

      uint32_t fileNum = 1;
      uint64_t sequenceNumber = 1;
      uint64_t const kMaxFileSize = 2 * 1024 * 1024;
      auto batch = make_shared<WriteBatch>();

      ::ThreadPool pool(concurrency);
      pool.init();

      deque<future<void>> futures;

      while (true) {
        unique_lock<std::mutex> lk(fMut);
        fCv.wait(lk, [this] { return !fQueue.empty() || fStop.load(); });
        vector<pair<string, string>> commands;
        fQueue.swap(commands);
        bool const stop = fStop.load();
        lk.unlock();
        for (auto const &c : commands) {
          auto const &key = c.first;
          auto const &value = c.second;
          InternalKey ik(key, sequenceNumber, kTypeValue);
          sequenceNumber++;
          Slice k = ik.Encode();
          batch->Put(k, value);
          if (batch->ApproximateSize() > o.max_file_size) {
            if (futures.size() > concurrency) {
              size_t pop = futures.size() - (size_t)(concurrency / 2);
              for (size_t i = 0; i < pop; i++) {
                futures.front().get();
                futures.pop_front();
              }
            }

            futures.emplace_back(move(pool.submit([this, fileNum, o](shared_ptr<WriteBatch> b) { drainWriteBatch(*b, fileNum, o); }, batch)));

            fileNum++;
            batch = make_shared<WriteBatch>();
          }
        }
        if (stop) {
          break;
        }
      }

      for (auto &f : futures) {
        f.get();
      }

      pool.shutdown();

      if (!fAbandoned) {
        drainWriteBatch(*batch, fileNum, o);
      }

      assert(fFileLock);
      auto env = leveldb::Env::Default();
      env->UnlockFile(fFileLock);
      fFileLock = nullptr;

      if (!fAbandoned) {
        RepairDB(fDir.c_str(), o);
        DB *db = nullptr;
        Status st = DB::Open(o, fDir, &db);
        if (st.ok()) {
          db->CompactRange(nullptr, nullptr);
          delete db;
        }
      }
    });
    fTh.swap(th);
  }

  bool valid() const override { return fValid; }

  void put(std::string const &key, leveldb::Slice const &value) override {
    auto cmd = std::make_pair(key, value.ToString());
    {
      std::lock_guard<std::mutex> lk(fMut);
      fQueue.push_back(cmd);
    }
    fCv.notify_one();
  }

  void del(std::string const &key) override {}

  ~RawDb() {
    fStop.store(true);
    fCv.notify_one();
    fTh.join();
  }

  void abandon() override {
    fAbandoned.store(true);
  }

private:
  void drainWriteBatch(leveldb::WriteBatch &b, uint32_t fileNum, leveldb::Options o) {
    using namespace leveldb;
    using namespace std;
    unique_ptr<WritableFile> file(open(fileNum));
    unique_ptr<TableBuilder> tb(new TableBuilder(o, file.get()));
    struct Visitor : public WriteBatch::Handler {
      void Put(const Slice &key, const Slice &value) override {
        fDrain.insert(make_pair(key.ToString(), value.ToString()));
        fKeys.push_back(key.ToString());
      }
      void Delete(const Slice &key) override {}

      std::unordered_map<std::string, std::string> fDrain;
      std::vector<std::string> fKeys;
    } v;
    b.Iterate(&v);
    sort(v.fKeys.begin(), v.fKeys.end(), [](string const &lhs, string const &rhs) { return BytewiseComparator()->Compare(lhs, rhs) < 0; });
    for (auto const &k : v.fKeys) {
      tb->Add(k, v.fDrain[k]);
    }
    tb->Finish();
    file->Close();
    b.Clear();
  }

  std::filesystem::path tableFile(uint32_t tableNumber) const {
    std::vector<char> buffer(11, (char)0);
#if defined(_WIN32)
    sprintf_s(buffer.data(), buffer.size(), "%06d.ldb", tableNumber);
#else
    sprintf(buffer.data(), "%06d.ldb", tableNumber);
#endif
    std::string p(buffer.data(), 10);
    return fDir / p;
  }

  leveldb::WritableFile *open(uint32_t tableNumber) const {
    using namespace leveldb;
    Env *env = Env::Default();
    WritableFile *f = nullptr;
    auto fname = tableFile(tableNumber);
    Status s = env->NewWritableFile(fname, &f);
    if (!s.ok()) {
      return nullptr;
    }
    return f;
  }

private:
  std::thread fTh;
  std::mutex fMut;
  bool fReady = false;
  std::vector<std::pair<std::string, std::string>> fQueue;
  std::condition_variable fCv;
  std::atomic_bool fStop;
  bool fValid = false;
  std::filesystem::path const fDir;
  leveldb::FileLock *fFileLock = nullptr;
  std::atomic_bool fAbandoned;
};

} // namespace je2be::tobe
