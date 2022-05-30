#include <stdio.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include <nlohmann/json.hpp>

namespace {

static std::vector<std::string> Split(std::string const &sentence, char delimiter) {
  std::istringstream input(sentence);
  std::vector<std::string> tokens;
  for (std::string line; std::getline(input, line, delimiter);) {
    tokens.push_back(line);
  }
  return tokens;
}

inline std::string LTrim(std::string const &s, std::string const &left) {
  if (left.empty()) {
    return s;
  }
  std::string ret = s;
  while (ret.starts_with(left)) {
    ret = ret.substr(left.size());
  }
  return ret;
}

inline std::string RTrim(std::string const &s, std::string const &right) {
  if (right.empty()) {
    return s;
  }
  std::string ret = s;
  while (ret.ends_with(right)) {
    ret = ret.substr(0, ret.size() - right.size());
  }
  return ret;
}

inline std::string Trim(std::string const &left, std::string const &s, std::string const &right) { return RTrim(LTrim(s, left), right); }

inline std::string Replace(std::string const &target, std::string const &search, std::string const &replace) {
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

static std::string Indent(std::string const &input, std::string const &indentString) {
  auto lines = Split(input, '\n');
  std::ostringstream ss;
  for (auto const &line : lines) {
    ss << indentString << line << std::endl;
  }
  return RTrim(RTrim(ss.str(), " "), "\n");
}

static std::string Trim(std::string const &v) {
  return Trim("\n", Trim(" ", v, " "), "\n");
}

std::string classFormat = R"(
#pragma once

namespace je2be::via {

class BackwardConverter {
  BackwardConverter() = delete;

public:
  enum class Version : uint8_t {
@{enum_definition}
  };
};

} // namespace je2be::via
)";

std::string tableFunctionFormat = R"(
static std::shared_ptr<mcfile::je::Block const> Convert@{version_pair}(std::shared_ptr<mcfile::je::Block const> const& input) {
  using namespace std;
  static unique_ptr<unordered_map<string, CompoundTagPtr> const> sDedicatedConvertMap(CreateDedicatedConvertMap@{version_pair}());
  static unique_ptr<unordered_map<string, string> const> sRenameConvertMap(CreateRenameConvertMap@{version_pair}());

  if (!input) {
    return nullptr;
  }

  auto const& dedicated = *sDedicatedConvertMap;
  auto const& rename = *sRenameConvertMap;

  if (auto found = rename.find(input->fName); found != rename.end()) {
    map<string, string> props(input->fProperties);
    return make_shared<mcfile::je::Block const>(found->second, props);
  }

  auto str = input->toString();
  if (auto found = dedicated.find(str); found != dedicated.end()) {
    return mcfile::je::Block::FromCompoundTag(*found->second);
  }

  return input;
}

static std::unordered_map<std::string, CompoundTagPtr> const* CreateDedicatedConvertMap@{version_pair}() {
  using namespace std;
  auto t = new unordered_map<string, CompoundTagPtr>();
  auto &s = *t;
@{dedicated_maps}
  return t;
}

static std::unordered_map<std::string, std::string> const* CreateRenameConvertMap@{version_pair}() {
  using namespace std;
  auto t = new unordered_map<string, string>();
  auto &s = *t;
@{rename_maps}
  return t;
}
)";
} // namespace

class Version {
public:
  explicit Version(std::string const &v) {
    auto tokens = Split(v, '.');
    if (tokens.size() < 2) {
      throw std::runtime_error("invalid version format: " + v);
    }
    fMajor = std::stoi(tokens[0]);
    fMinor = std::stoi(tokens[1]);
    if (tokens.size() > 2) {
      fBugfix = std::stoi(tokens[2]);
    }
    if (tokens.size() > 3) {
      throw std::runtime_error("invalid version format: " + v);
    }
  }

  std::string toString() const {
    std::string ret = std::to_string(fMajor) + "." + std::to_string(fMinor);
    if (fBugfix) {
      ret += "." + std::to_string(*fBugfix);
    }
    return ret;
  }

  bool equals(Version const &other) const {
    if (fMajor != other.fMajor) {
      return false;
    }
    if (fMinor != other.fMinor) {
      return false;
    }
    if (!fBugfix && !other.fBugfix) {
      return true;
    }
    if (fBugfix && other.fBugfix) {
      return *fBugfix == *other.fBugfix;
    } else {
      return false;
    }
  }

  int fMajor;
  int fMinor;
  std::optional<int> fBugfix;
};

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

  vector<pair<Version, Version>> versionPairs;
  set<string> versions;

  for (auto item : fs::directory_iterator(directory)) {
    if (!item.is_regular_file()) {
      continue;
    }
    auto p = item.path();
    auto filename = p.filename();
    if (filename.extension().string() != ".json" || !filename.string().starts_with("mapping-")) {
      continue;
    }
    auto pair = filename.replace_extension().string().substr(8);
    auto indexTo = pair.find_first_of("to");
    if (indexTo == string::npos) {
      throw runtime_error("invalid version pair format: " + pair);
    }
    auto toVersionString = pair.substr(0, indexTo);
    auto fromVersionString = pair.substr(indexTo + 2);
    Version to(toVersionString);
    Version from(fromVersionString);
    versionPairs.push_back(make_pair(from, to)); // filename: 1.18to1.19, make_pair(1.19, 1.18)
    versions.insert(from.toString());
    versions.insert(to.toString());
  }

  sort(versionPairs.begin(), versionPairs.end(), [](auto const &pairA, auto const &pairB) {
    Version const &a = pairA.first;
    Version const &b = pairB.first;

    if (a.fMajor == b.fMajor) {
      if (a.fMinor == b.fMinor) {
        if (a.fBugfix && b.fBugfix) {
          return *a.fBugfix > *b.fBugfix;
        } else if (a.fBugfix) {
          return true;
        } else {
          return false;
        }
      } else {
        return a.fMinor > b.fMinor;
      }
    } else {
      return a.fMajor > b.fMajor;
    }
  });

  for (int i = 0; i < versionPairs.size() - 1; i++) {
    auto from = versionPairs[i].second;
    auto to = versionPairs[i + 1].first;
    if (!from.equals(to)) {
      throw runtime_error("version numbers are not consecutive");
    }
  }

  int enumValue = 1;
  ostringstream enumDefinition;
  for (auto const &v : versionPairs) {
    enumDefinition << "Version" << Replace(v.first.toString(), ".", "_") << " = " << enumValue << "," << endl;
    enumValue++;
  }
  enumDefinition << "Version" << Replace(versionPairs.back().second.toString(), ".", "_") << " = " << enumValue << "," << endl;

  ofstream out(argv[1]);
  string src = Replace(classFormat, "@{enum_definition}", Indent(Trim(enumDefinition.str()), "    "));
  out << Trim(src) << endl;

  return 0;
}
