#pragma once

namespace je2be {

class Volume {
public:
  Volume(Pos3i start, Pos3i end) : fStart(start), fEnd(end) {}

  static std::optional<Volume> Intersection(Volume const &a, Volume const &b) {
    auto x = Intersection(a.fStart.fX, a.fEnd.fX, b.fStart.fX, b.fEnd.fX);
    auto y = Intersection(a.fStart.fY, a.fEnd.fY, b.fStart.fY, b.fEnd.fY);
    auto z = Intersection(a.fStart.fZ, a.fEnd.fZ, b.fStart.fZ, b.fEnd.fZ);
    if (!x || !y || !z) {
      return std::nullopt;
    }
    auto [x0, x1] = *x;
    auto [y0, y1] = *y;
    auto [z0, z1] = *z;
    Pos3i start(x0, y0, z0);
    Pos3i end(x1, y1, z1);
    return Volume(start, end);
  }

  static bool Equals(Volume const &a, Volume const &b) { return a.fStart == b.fStart && a.fEnd == b.fEnd; }

  std::shared_ptr<Tag> toNbt() const {
    using namespace std;
    using namespace mcfile;
    auto tag = make_shared<CompoundTag>();
    (*tag)["start"] = Pos3iToNbt(fStart);
    (*tag)["end"] = Pos3iToNbt(fEnd);
    return tag;
  }

  static std::optional<Volume> FromNbt(Tag const &tag) {
    using namespace std;
    using namespace mcfile;
    auto c = tag.asCompound();
    if (!c) {
      return nullopt;
    }
    auto startTag = c->compoundTag("start");
    auto endTag = c->compoundTag("end");
    if (!startTag || !endTag) {
      return nullopt;
    }
    auto start = Pos3iFromNbt(*startTag);
    auto end = Pos3iFromNbt(*endTag);
    if (!start || !end) {
      return nullopt;
    }
    return Volume(*start, *end);
  }

  static void Connect(std::vector<Volume> &volumes) {
    using namespace std;
    while (true) {
      int before = volumes.size();
      CollectAndConnect<0, 1, 2>(volumes);
      CollectAndConnect<0, 2, 1>(volumes);
      CollectAndConnect<1, 2, 0>(volumes);
      if (before == volumes.size()) {
        break;
      }
    }
  }

  template <size_t dimension> // = 0 for X, = 1 for Y, = 2 for Z
  int32_t size() const {
    return end<dimension>() - start<dimension>() + 1;
  }

  template <size_t dimension>
  int32_t start() const {
    static_assert(dimension < 3);
    switch (dimension) {
    case 0:
      return fStart.fX;
    case 1:
      return fStart.fY;
    case 2:
      return fStart.fZ;
    default:
      return 0;
    }
  }

  template <size_t dimension>
  int32_t end() const {
    static_assert(dimension < 3);
    switch (dimension) {
    case 0:
      return fEnd.fX;
    case 1:
      return fEnd.fY;
    case 2:
      return fEnd.fZ;
    default:
      return 0;
    }
  }

private:
  template <size_t d1, size_t d2, size_t d3>
  static void CollectAndConnect(std::vector<Volume> &volumes) {
    static_assert(d1 < 3 && d2 < 3 && d3 < 3 && d1 != d2 && d2 != d3 && d1 != d3);
    using namespace std;
    vector<Volume> tmp;
    vector<vector<Volume>> buffer;
    CollectCuboidWithSamePlane<d1, d2>(volumes, buffer);
    for (auto &collected : buffer) {
      ConnectCuboidWithSamePlane<d3>(collected);
      copy(collected.begin(), collected.end(), back_inserter(tmp));
    }
    tmp.swap(volumes);
  }

  template <size_t d1, size_t d2>
  static void CollectCuboidWithSamePlane(std::vector<Volume> const &input, std::vector<std::vector<Volume>> &output) {
    static_assert(d1 < 3 && d2 < 3 && d1 != d2);
    using namespace std;
    map<tuple<int32_t, int32_t, int32_t, int32_t>, vector<Volume>> categorized;
    for (Volume const &v : input) {
      int32_t s1 = v.start<d1>();
      int32_t e1 = v.end<d1>();
      int32_t s2 = v.start<d2>();
      int32_t e2 = v.end<d2>();
      tuple<int32_t, int32_t, int32_t, int32_t> key(s1, e1, s2, e2);
      categorized[key].push_back(v);
    }
    output.clear();
    for (auto const &it : categorized) {
      output.push_back(it.second);
    }
  }

  template <size_t d3>
  static void ConnectCuboidWithSamePlane(std::vector<Volume> &inout) {
    static_assert(d3 < 3);
    using namespace std;
    while (true) {
      vector<Volume> buffer;
      unordered_set<int> ignore;
      for (int i = 0; i < inout.size() - 1; i++) {
        if (ignore.find(i) != ignore.end()) {
          continue;
        }
        Volume v1 = inout[i];
        int32_t s1 = v1.start<d3>();
        int32_t e1 = v1.end<d3>();
        for (int j = i + 1; j < inout.size(); j++) {
          if (ignore.find(j) != ignore.end()) {
            continue;
          }
          Volume v2 = inout[j];
          int32_t s2 = v2.start<d3>();
          int32_t e2 = v2.end<d3>();
          if (e1 + 1 == s2 || e2 + 1 == s1) {
            Pos3i start((min)(v1.start<0>(), v2.start<0>()), (min)(v1.start<1>(), v2.start<1>()), (min)(v1.start<2>(), v2.start<2>()));
            Pos3i end((max)(v1.end<0>(), v2.end<0>()), (max)(v1.end<1>(), v2.end<1>()), (max)(v1.end<2>(), v2.end<2>()));
            Volume connected(start, end);
            buffer.push_back(connected);
            ignore.insert(i);
            ignore.insert(j);
            break;
          }
        }
      }
      for (int i = 0; i < inout.size(); i++) {
        if (ignore.find(i) == ignore.end()) {
          buffer.push_back(inout[i]);
        }
      }
      if (buffer.size() == inout.size()) {
        break;
      }
      buffer.swap(inout);
    }
  }

  static std::optional<std::tuple<int32_t, int32_t>> Intersection(int32_t a0, int32_t a1, int32_t b0, int32_t b1) {
    using namespace std;
    int32_t maxLowerBound = std::max(a0, b0);
    int32_t minUpperBound = std::min(a1, b1);
    if (maxLowerBound < minUpperBound) {
      return make_tuple(maxLowerBound, minUpperBound);
    } else {
      return nullopt;
    }
  }

public:
  Pos3i fStart;
  Pos3i fEnd;
};

} // namespace je2be
