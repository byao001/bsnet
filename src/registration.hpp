//
// Created by byao on 12/8/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_REGISTRATION_HPP
#define BSNET_REGISTRATION_HPP

#include "event.hpp"
#include "poller_epoll.hpp"
#include "utility.hpp"

namespace bsnet {

class ReadinessQueue;
class SetReadiness;

class Registration : public Evented, public NonCopyable {
  struct InnerRegistration : public Evented {
    friend class Registration;
    friend class SetReadiness;

    void register_on(Poller &poller, Token tok, Ready interest,
                     PollOpt opts) override;
    void reregister_on(Poller &poller, Token tok, Ready interest,
                       PollOpt opts) override;
    void deregister_on(Poller &poller) override;
    ~InnerRegistration() noexcept override = default;

    void set_readiness(Ready r);

  private:
    ReadinessQueue *_rq;
    Event _evt;
  };

  static inline ReadinessQueue *rq(Poller &poller) { return poller.rq(); }

public:
  friend class SetReadiness;

  Registration();
  SetReadiness new_set_readiness() const;

  void register_on(Poller &poller, Token tok, Ready interest,
                   PollOpt opts) override;
  void reregister_on(Poller &poller, Token tok, Ready interest,
                     PollOpt opts) override;
  void deregister_on(Poller &poller) override;
  ~Registration() noexcept override = default;

private:
  InnerRegistration *_inner_node;
};

class SetReadiness : public NonCopyable {
public:
  friend class Registration;
  SetReadiness(SetReadiness &&other) noexcept;
  ~SetReadiness() noexcept;
  void swap(SetReadiness &other) noexcept;

  void set_readiness(Ready r);

private:
  SetReadiness() = default;
  Registration::InnerRegistration *_inner_node;
};
}

#endif // !BSNET_REGISTRATION_HPP