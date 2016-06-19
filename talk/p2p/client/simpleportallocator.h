#ifndef _SimplePortAllocator_H_
#define _SimplePortAllocator_H_

#include "talk/p2p/client/basicportallocator.h"

namespace talk_base {
  class SignalThread;
}

namespace cricket {

class SimplePortAllocator : public BasicPortAllocator {
public:
  SimplePortAllocator(talk_base::NetworkManager* network_manager, const std::string &user_agent);
  virtual ~SimplePortAllocator();

  virtual PortAllocatorSession *CreateSession(const std::string &name,
					      const std::string &session_type);
  void SetStunHosts(const std::vector<talk_base::SocketAddress> &hosts) {stun_hosts_ = hosts;}
  void SetRelayHosts(const std::vector<std::string> &hosts) {relay_hosts_ = hosts;}
  void SetRelayToken(const std::string &relay) {relay_token_ = relay;}
  std::string relay_token() const { return relay_token_; }
private:
  std::vector<talk_base::SocketAddress> stun_hosts_;
  std::vector<std::string> relay_hosts_;
  std::string relay_token_;
  std::string agent_;
};

class RequestData;

class SimplePortAllocatorSession : public BasicPortAllocatorSession {
 public:
  SimplePortAllocatorSession(SimplePortAllocator *allocator,
                           const std::string &name,
			   const std::string &session_type,
			   const std::vector<talk_base::SocketAddress> &stun_hosts,
			   const std::vector<std::string> &relay_hosts,
			   const std::string &relay,
			   const std::string &agent);
  ~SimplePortAllocatorSession() {};

protected:
  virtual void GetPortConfigurations();

private:
  std::vector<std::string> relay_hosts_;
  std::vector<talk_base::SocketAddress> stun_hosts_;
  std::string relay_token_;
  std::string agent_;

  SimplePortAllocator* http_allocator() {
    return static_cast<SimplePortAllocator*>(allocator());
  }
  int attempts_;
};

} // namespace cricket

#endif // _XMPPPORTALLOCATOR_H_
