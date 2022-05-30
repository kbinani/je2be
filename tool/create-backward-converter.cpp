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

class Backward {
  Backward() = delete;

public:
  enum class Version : uint8_t {
@{enum_definition}
  };

  using Converter = std::function<std::shared_ptr<mcfile::je::Block const>(std::shared_ptr<mcfile::je::Block const> const&)>;

  static Converter ComposeConverter(Version from, Version to) {
    CompositeConverter composite;
    std::vector<Converter> converters;
    for (uint8_t v = static_cast<uint8_t>(from); v < static_cast<uint8_t>(to); v++) {
      auto converter = SelectConverter(static_cast<Version>(v));
      if (!converter) {
        return composite;
      }
      converters.push_back(converter);
    }
    composite.fConverters.swap(converters);
    return composite;
  }

private:
  struct CompositeConverter {
    std::vector<Converter> fConverters;

    std::shared_ptr<mcfile::je::Block const> operator() (std::shared_ptr<mcfile::je::Block const> const& input) const {
      auto output = input;
      for (auto const& converter : fConverters) {
        output = converter(output);
      }
      return output;
    }
  };

  static CompoundTagPtr CompoundTagFromDataString(std::string const &data) {
    using namespace std;
    auto found = data.find('[');
    auto tag = Compound();
    if (found != string::npos && data.ends_with(']')) {
      auto props = Compound();
      tag->set("Name", String(data.substr(0, found)));
      auto s = data.substr(found, data.size() - found - 1);
      istringstream input(s);
      for (string kv; getline(input, kv, ',');) {
        auto equal = kv.find('=');
        if (equal == string::npos) {
          continue;
        }
        auto key = kv.substr(0, equal);
        auto value = kv.substr(equal + 1);
        props->set(key, String(value));
      }
      tag->set("Properties", props);
    } else {
      tag->set("Name", String(data));
    }
    return tag;
  }

@{convert_function_definitions}

  static Converter SelectConverter(Version from) {
    static std::unordered_map<Version, Converter> const sConverters = {
@{converters_table}
    };
    if (auto found = sConverters.find(from); found != sConverters.end()) {
      return found->second;
    } else {
      return nullptr;
    }
  }
};

} // namespace je2be::via
)";

std::string emptyTableFunctionFormat = R"(
static std::shared_ptr<mcfile::je::Block const> Convert@{version_pair}(std::shared_ptr<mcfile::je::Block const> const& input) {
  return input;
}
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

  std::string toString(std::string separator = ".") const {
    std::string ret = std::to_string(fMajor) + separator + std::to_string(fMinor);
    if (fBugfix) {
      ret += separator + std::to_string(*fBugfix);
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

  bool less(Version const &b) const {
    if (fMajor == b.fMajor) {
      if (fMinor == b.fMinor) {
        if (fBugfix && b.fBugfix) {
          return *fBugfix > *b.fBugfix;
        } else if (fBugfix) {
          return true;
        } else {
          return false;
        }
      } else {
        return fMinor > b.fMinor;
      }
    } else {
      return fMajor > b.fMajor;
    }
  }

  int fMajor;
  int fMinor;
  std::optional<int> fBugfix;
};

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace nlohmann;
  namespace fs = std::filesystem;

  if (argc < 2) {
    return -1;
  }

  fs::path directory(argv[2]);
  if (!fs::is_directory(directory)) {
    return -1;
  }

  vector<pair<Version, Version>> versionPairs;
  ostringstream convertFunctionDefinitions;
  ostringstream convertersTable;

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

    string versionPair = from.toString("_") + "To" + to.toString("_");

    convertersTable << "{Version::Version" << from.toString("_") << ", Convert" << versionPair << "}," << endl;

    ifstream ifs(p.string());
    auto obj = json::parse(ifs);
    auto found = obj.find("blockstates");
    if (found == obj.end()) {
      convertFunctionDefinitions << Replace(emptyTableFunctionFormat, "@{version_pair}", versionPair);
    } else {
      json &blockstates = found.value();
      ostringstream dedicated;
      ostringstream rename;
      for (json::iterator it = blockstates.begin(); it != blockstates.end(); it++) {
        string key = it.key();
        if (key.ends_with("[")) {
          cout << "warning: eratta in key?: key=" << key << endl;
          key = key.substr(0, key.size() - 1);
        }
        string value = it.value().get<string>();
        if (value.ends_with("[")) {
          rename << "s[\"" << key << "\"] = \"" << value.substr(0, value.size() - 1) << "\";" << endl;
        } else {
          dedicated << "s[\"" << key << "\"] = CompoundTagFromDataString(\"" << value << "\");" << endl;
        }
      }
      string src = Replace(tableFunctionFormat, "@{rename_maps}", Indent(rename.str(), "  "));
      src = Replace(src, "@{dedicated_maps}", Indent(dedicated.str(), "  "));
      src = Replace(src, "@{version_pair}", versionPair);
      convertFunctionDefinitions << src;
    }
  }

  sort(versionPairs.begin(), versionPairs.end(), [](auto const &pairA, auto const &pairB) {
    return pairA.first.less(pairB.first);
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
    enumDefinition << "Version" << v.first.toString("_") << " = " << enumValue << "," << endl;
    enumValue++;
  }
  enumDefinition << "Version" << versionPairs.back().second.toString("_") << " = " << enumValue << "," << endl;

  ofstream out(argv[1]);
  string src = Replace(classFormat, "@{enum_definition}", Indent(Trim(enumDefinition.str()), "    "));
  src = Replace(src, "@{convert_function_definitions}", Indent(Trim(convertFunctionDefinitions.str()), "  "));
  src = Replace(src, "@{converters_table}", Indent(Trim(convertersTable.str()), "      "));
  out << Trim(src) << endl;

  return 0;
}
