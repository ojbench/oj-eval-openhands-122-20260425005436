#ifndef SRC_HPP
#define SRC_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>

#include "event.h"

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  std::string base = NotifyLateEvent::GetNotification(n);
  return base + (generator_ ? generator_(n) : std::string());
}

class Memo {
 public:
  Memo() = delete;
  explicit Memo(int duration) : duration_(duration), cur_time_(0) {}
  ~Memo() = default;

  void AddEvent(const Event *event) {
    if (!event) return;
    // Schedule notifications within [1, duration_]
    int d = event->GetDeadline();
    if (d >= 1 && d <= duration_) {
      if (dynamic_cast<const NotifyBeforeEvent *>(event)) {
        schedule_[d].push_back({event, 1});
      } else {
        schedule_[d].push_back({event, 0});
      }
    }

    if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(event)) {
      int pre_t = d - nb->GetNotifyTime() + 1;
      if (pre_t >= 1 && pre_t <= duration_) {
        schedule_[pre_t].push_back({event, 0});
      }
      return;
    }

    if (auto nl = dynamic_cast<const NotifyLateEvent *>(event)) {
      int freq = nl->GetFrequency();
      if (freq > 0) {
        for (int t = d + freq, n = 1; t <= duration_; t += freq, ++n) {
          schedule_[t].push_back({event, n});
        }
      }
    }
  }

  void Tick() {
    if (cur_time_ >= duration_) return;
    ++cur_time_;
    auto it = schedule_.find(cur_time_);
    if (it == schedule_.end()) return;
    auto &vec = it->second;
    auto priority = [](const Event *ev, int n) {
      // 0: NB pre-notify (n==0)
      // 1: NB deadline (n==1)
      // 2: Normal deadline (n==0)
      // 3: NL deadline (n==0)
      // 4: NL late (n>0)
      if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(ev)) {
        if (n == 0) return 0;
        if (n == 1) return 1;
      }
      if (auto nl = dynamic_cast<const NotifyLateEvent *>(ev)) {
        if (n == 0) return 3;
        if (n > 0) return 4;
      }
      // NormalEvent deadline
      return 2;
    };
    stable_sort(vec.begin(), vec.end(), [&](const auto &a, const auto &b){
      return priority(a.first, a.second) < priority(b.first, b.second);
    });
    for (auto &p : vec) {
      const Event *ev = p.first;
      int n = p.second;
      if (!ev || ev->IsComplete()) continue;
      std::cout << ev->GetNotification(n) << '\n';
    }
  }

 private:
  int duration_;
  int cur_time_;
  std::unordered_map<int, std::vector<std::pair<const Event *, int>>> schedule_;
};

#endif // SRC_HPP
