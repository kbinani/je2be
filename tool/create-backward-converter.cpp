#include <stdio.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace {
std::string Replace(std::string const &target, std::string const &search, std::string const &replace) {
  using namespace std;
  if (search.empty()) {
    return target;
  }
  if (search == replace) {
    return target;
  }
  size_t offset = 0;
  string ret = target;
  while (true) {
    auto found = ret.find(search, offset);
    if (found == string::npos) {
      break;
    }
    ret = ret.substr(0, found) + replace + ret.substr(found + search.size());
    offset = found + replace.size();
  }
  return ret;
}
} // namespace

int main(int argc, char *argv[]) {
  using namespace std;
  namespace fs = std::filesystem;

  if (argc < 1) {
    return -1;
  }
  ofstream out(argv[1]);
  if (!out) {
    return -1;
  }

  // TODO: Parse json and create c++ class

  out << "// clang-format off" << endl;

  vector<char> buffer(256);
  for (int i = 2; i < argc; i++) {
    cout << "reading " << argv[i] << endl;
    FILE *fp = fopen(argv[i], "rb");
    if (!fp) {
      return -1;
    }
    fs::path file(argv[i]);
    string name = file.filename().string();
    name = Replace(name, ".", "_");
    name = Replace(name, "-", "_");

    out << "char const " << name << "[] = {" << endl;
    size_t size = 0;
    while (!feof(fp)) {
      size_t read = fread(buffer.data(), 1, buffer.size(), fp);
      out << "  ";
      for (size_t j = 0; j < read; j++) {
        out << "0x" << hex << (int)buffer[j] << ", ";
      }
      size += read;
      out << endl;
    }
    fclose(fp);
    out << "};" << endl;
    out << dec << "size_t const " << name << "_bytes = " << size << ";" << endl;
  }
  return 0;
}
