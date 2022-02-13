#pragma once

namespace je2be::file {

static inline std::optional<std::filesystem::path> CreateTempDir(std::filesystem::path const &tempDir) {
  namespace fs = std::filesystem;
  auto tmp = fs::temp_directory_path();
#if defined(_MSC_VER)
  wchar_t *dir = _wtempnam(tmp.native().c_str(), L"j2b-tmp-");
  if (dir) {
    fs::path ret(dir);
    fs::create_directory(ret);
    free(dir);
    return ret;
  } else {
    return std::nullopt;
  }
#else
  using namespace std;
  string tmpl("j2b-tmp-XXXXXX");
  vector<char> buffer;
  copy(tmpl.begin(), tmpl.end(), back_inserter(buffer));
  buffer.push_back(0);
  char *dir = mkdtemp(buffer.data());
  if (dir) {
    return string(dir, strlen(dir));
  } else {
    return nullopt;
  }
#endif
}

[[nodiscard]] static inline bool GetContents(std::filesystem::path p, std::vector<uint8_t> &buffer) {
  using namespace std;
  using namespace mcfile;
  if (!Fs::Exists(p)) {
    return false;
  }
  auto size = Fs::FileSize(p);
  if (!size) {
    return false;
  }
  ScopedFile fp(mcfile::File::Open(p, mcfile::File::Mode::Read));
  if (!fp) {
    return false;
  }
  buffer.resize(*size);
  if (!File::Fread(buffer.data(), *size, 1, fp)) {
    return false;
  }
  return true;
}

[[nodiscard]] static inline bool GetGzContents(std::filesystem::path p, std::vector<uint8_t> &buffer) {
  buffer.clear();
  std::vector<char> buf(512);
  gzFile f = mcfile::File::GzOpen(p, mcfile::File::Mode::Read);
  if (!f) {
    return false;
  }
  while (true) {
    int read = gzread(f, buf.data(), buf.size());
    if (read <= 0) {
      break;
    }
    std::copy_n(buf.begin(), read, back_inserter(buffer));
    if (read < buf.size()) {
      break;
    }
  }
  return gzclose_r(f) == Z_OK;
}

} // namespace je2be::file
