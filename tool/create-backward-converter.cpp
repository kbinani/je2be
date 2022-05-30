#include <stdio.h>

#include <nlohmann/json.hpp>

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

  if (argc < 2) {
    return -1;
  }

  fs::path directory(argv[2]);
  if (!fs::is_directory(directory)) {
    return -1;
  }

  ofstream out(argv[1]);
  out << "#pragma once" << endl;
  out << endl;
  out << "namespace je2be::via {" << endl;
  out << endl;
  out << "class BackwardConverter {" << endl;
  out << "  BackwardConverter() = delete;" << endl;

  for (auto item : fs::directory_iterator(directory)) {
    if (!item.is_regular_file()) {
      continue;
    }
    auto p = item.path();
    auto filename = p.filename();
    if (filename.extension().string() != ".json" || !filename.string().starts_with("mapping-")) {
      continue;
    }
    cout << p << endl;
    ifstream ifs(p.string());
    auto obj = nlohmann::json::parse(ifs);
    auto found = obj.find("blockstates");
    if (found == obj.end()) {
      continue;
    }
    auto const &blockstates = found.value();
    cout << blockstates << endl;
  }

  out << "};" << endl;
  out << endl;
  out << "} // namespace::via" << endl;

  return 0;
}
