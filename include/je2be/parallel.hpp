#pragma once

#include <latch>
#include <thread>

namespace je2be {

class Parallel {
public:
  template <class Work, class Result>
  static void Map(
      std::vector<Work> const &works,
      std::function<Result(Work const &work, int index)> func,
      std::vector<Result> &out) {
    using namespace std;

    auto threads = thread::hardware_concurrency();
    std::latch latch(threads + 1);

    out.resize(works.size());
    Result *outPtr = out.data();
    Work const *worksPtr = works.data();

    mutex mut;
    vector<bool> done(works.size(), false);

    for (int i = 0; i < threads; i++) {
      thread([&latch, worksPtr, outPtr, &mut, &done, func]() {
        while (true) {
          int index = -1;
          {
            lock_guard<mutex> lock(mut);
            for (int j = 0; j < done.size(); j++) {
              if (!done[j]) {
                index = j;
                done[j] = true;
                break;
              }
            }
          }
          if (index < 0) {
            break;
          } else {
            outPtr[index] = func(worksPtr[index], index);
          }
        }
        latch.count_down();
      }).detach();
    }
    latch.arrive_and_wait();
  }

  template <class Work, class Result>
  static Result Reduce(
      std::vector<Work> const &works,
      Result init,
      std::function<Result(Work const &)> func,
      std::function<void(Result const &, Result &)> join) {
    using namespace std;

    auto threads = thread::hardware_concurrency();
    std::latch latch(threads + 1);

    mutex queueMut;
    vector<bool> done(works.size(), false);

    mutex joinMut;
    Result total = init;

    for (int i = 0; i < threads; i++) {
      thread([init, &latch, &queueMut, &joinMut, &works, func, join, &total, &done]() {
        Result sum = init;
        while (true) {
          Work const *work = nullptr;
          {
            lock_guard<mutex> lock(queueMut);
            for (int j = 0; j < works.size(); j++) {
              if (!done[j]) {
                work = &works[j];
                done[j] = true;
                break;
              }
            }
          }
          if (work) {
            Result result = func(*work);
            join(result, sum);
          } else {
            lock_guard<mutex> lock(joinMut);
            join(sum, total);
            break;
          }
        }
        latch.count_down();
      }).detach();
    }
    latch.arrive_and_wait();
    return total;
  }

  static void Merge(bool const &from, bool &to) {
    to = from && to;
  }
};

} // namespace je2be