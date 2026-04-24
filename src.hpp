#ifndef SRC_HPP
#define SRC_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include "event.h"

// Implement CustomNotifyLateEvent::GetNotification as specified
inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  std::string base = NotifyLateEvent::GetNotification(n);
  return base + (generator_ ? generator_(n) : std::string());
}

class Memo {
 public:
  Memo() = delete;
  explicit Memo(int duration) : duration_(duration), cur_time_(0) {}
  ~Memo() = default;

  void AddEvent(const Event *event) { events_.push_back(event); }

  // Advance one hour and emit notifications for that hour
  void Tick() {
    if (cur_time_ >= duration_) return;
    ++cur_time_;

    for (const Event *ev : events_) {
      if (!ev || ev->IsComplete()) continue;

      int t = cur_time_;
      int d = ev->GetDeadline();

      // Deadline notifications
      if (t == d) {
        if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(ev)) {
          std::cout << nb->GetNotification(1) << '\n';
          continue;
        }
        std::cout << ev->GetNotification(0) << '\n';
        continue;
      }

      // Pre-notify for NotifyBeforeEvent at time (deadline - notify_time + 1)
      if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(ev)) {
        int pre_t = d - nb->GetNotifyTime() + 1;
        if (t == pre_t) {
          std::cout << nb->GetNotification(0) << '\n';
        }
        continue;
      }

      // Late notifications for NotifyLateEvent and CustomNotifyLateEvent
      if (t > d) {
        if (auto nl = dynamic_cast<const NotifyLateEvent *>(ev)) {
          int freq = nl->GetFrequency();
          if (freq > 0) {
            int delta = t - d;
            if (delta % freq == 0) {
              int n = delta / freq;  // n >= 1 here
              std::cout << nl->GetNotification(n) << '\n';
            }
          }
        }
      }
    }
  }

 private:
  int duration_;
  int cur_time_;
  std::vector<const Event *> events_;
};

#endif // SRC_HPP
