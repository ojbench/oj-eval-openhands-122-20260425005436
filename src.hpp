#ifndef SRC_HPP
#define SRC_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;

// Event base and derived classes (from specification)
class Event {
public:
  Event() = delete;
  Event(const string &name, int deadline) : name_(name), deadline_(deadline), complete_(false) {}
  Event(const Event &) = delete;
  Event(Event &&) = delete;
  Event &operator=(const Event &) = delete;
  Event &operator=(Event &&) = delete;
  virtual ~Event() = default;

  const string &GetName() const { return name_; }
  void SetComplete() { complete_ = true; }
  bool IsComplete() const { return complete_; }
  int GetDeadline() const { return deadline_; }
  virtual string GetNotification(int n) const = 0;

private:
  string name_;
  int deadline_;
  bool complete_;
};

class NormalEvent final : public Event {
public:
  NormalEvent(const string &name, int deadline) : Event(name, deadline) {}
  string GetNotification(int n) const override {
    if (n != 0) {
      throw runtime_error("Notification argument is invalid for Normal Events!");
    }
    return string("Normal Event \"") + GetName() + "\" is over.";
  }
};

class NotifyBeforeEvent final : public Event {
public:
  NotifyBeforeEvent(const string &name, int deadline, int notify_time)
      : Event(name, deadline), notify_time_(notify_time) {}
  int GetNotifyTime() const { return notify_time_; }
  string GetNotification(int n) const override {
    if (n == 0) {
      return string("Notify Before Event \"") + GetName() + "\" is about to end. Please hurry!";
    }
    if (n == 1) {
      return string("Notify Before Event \"") + GetName() + "\" is over.";
    }
    throw runtime_error("Notification argument is invalid for Notify Before Events!");
  }

private:
  int notify_time_;
};

class NotifyLateEvent : public Event {
public:
  NotifyLateEvent(const string &name, int deadline, int frequency)
      : Event(name, deadline), frequency_(frequency) {}
  int GetFrequency() const { return frequency_; }
  string GetNotification(int n) const override {
    if (n == 0) {
      return string("Notify Late Event \"") + GetName() + "\" is over.";
    }
    if (n > 0) {
      return string("Notify Late Event \"") + GetName() + "\" is late for " + to_string(frequency_ * n) + " hours. ";
    }
    throw runtime_error("Notification argument is invalid for Notify Late Events!");
  }

private:
  int frequency_;
};

class CustomNotifyLateEvent final : public NotifyLateEvent {
public:
  CustomNotifyLateEvent(const string &name, int deadline, int frequency, string (*generator)(int))
      : NotifyLateEvent(name, deadline, frequency), generator_(generator) {}
  string GetNotification(int n) const override {
    // Must first call base class GetNotification(n), then append generator output
    string base_msg = NotifyLateEvent::GetNotification(n);
    string extra = generator_ ? generator_(n) : string();
    return base_msg + extra;
  }

private:
  string (*generator_)(int);
};

class Memo {
public:
  Memo() = delete;
  explicit Memo(int duration) : duration_(duration), cur_time_(0) {}
  ~Memo() = default;

  void AddEvent(const Event *event) { events_.push_back(event); }

  void Tick() {
    if (cur_time_ >= duration_) {
      // Beyond configured duration, do nothing
      return;
    }
    ++cur_time_;

    for (const Event *ev : events_) {
      if (!ev || ev->IsComplete()) continue;

      int t = cur_time_;
      int d = ev->GetDeadline();

      // Deadline notifications
      if (t == d) {
        // Normal or Late-type deadline
        // For NotifyBeforeEvent, deadline uses n=1; for others deadline uses n=0
        if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(ev)) {
          cout << nb->GetNotification(1) << '\n';
          continue;
        }
        cout << ev->GetNotification(0) << '\n';
        continue;
      }

      // Pre-notify for NotifyBeforeEvent at time (deadline - notify_time + 1)
      if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(ev)) {
        int pre_t = d - nb->GetNotifyTime() + 1;
        if (t == pre_t) {
          cout << nb->GetNotification(0) << '\n';
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
              int n = delta / freq; // n >= 1 here
              cout << nl->GetNotification(n) << '\n';
            }
          }
        }
      }
    }
  }

private:
  int duration_;
  int cur_time_;
  vector<const Event *> events_;
};

#endif // SRC_HPP
