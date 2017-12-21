#include "registration.hpp"
#include "poller_epoll.hpp"

namespace bsnet {

void Registration::InnerRegistration::register_on(Poller &poller, Token tok,
                                                  Ready interest,
                                                  PollOpt opts) {
  _rq = Registration::rq(poller);
  _evt.set_events(interest, opts);
  _evt.set_token(tok);
}

void Registration::InnerRegistration::reregister_on(Poller &poller, Token tok,
                                                    Ready interest,
                                                    PollOpt opts) {
  _rq = Registration::rq(poller);
  _evt.set_events(interest, opts);
  _evt.set_token(tok);
}

void Registration::InnerRegistration::deregister_on(Poller &poller) {
  _rq = nullptr;
}

void Registration::InnerRegistration::set_readiness(Ready r) {
  Ready evts = _evt.readiness();
  if (evts.contains(r) && _rq) {
    _rq->emplace(r, PollOpt::empty(), _evt.token());

    if (evts.contains(static_cast<uint32_t>(PollOpt::oneshot()))) {
      _rq = nullptr;
    }
  }
}

Registration::Registration() : _inner_node(new InnerRegistration()) {}

SetReadiness Registration::new_set_readiness() const {
  SetReadiness sr;
  sr._inner_node = _inner_node;
  return sr;
}

void Registration::register_on(Poller &poller, Token tok, Ready interest,
                               PollOpt opts) {
  _inner_node->register_on(poller, tok, interest, opts);
}

void Registration::reregister_on(Poller &poller, Token tok, Ready interest,
                                 PollOpt opts) {
  _inner_node->reregister_on(poller, tok, interest, opts);
}

void Registration::deregister_on(Poller &poller) {
  _inner_node->deregister_on(poller);
}

void SetReadiness::swap(SetReadiness &other) noexcept {
  using std::swap;
  swap(_inner_node, other._inner_node);
}

SetReadiness::SetReadiness(SetReadiness &&other) noexcept
    : _inner_node(nullptr) {
  this->swap(other);
}

SetReadiness::~SetReadiness() noexcept { delete _inner_node; }

void SetReadiness::set_readiness(Ready r) { _inner_node->set_readiness(r); }
}