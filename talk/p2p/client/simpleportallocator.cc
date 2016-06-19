#if defined(_MSC_VER) && _MSC_VER < 1300
#pragma warning(disable:4786)
#endif
#include "talk/base/basicdefs.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/signalthread.h"
#include "talk/p2p/client/simpleportallocator.h"
#include <cassert>
#include <ctime>

namespace {

// Records the port on the hosts that will receive HTTP requests.
const uint16 kHostPort = 80;

// Records the URL that we will GET in order to create a session.
const std::string kCreateSessionURL = "/create_session";

// The number of HTTP requests we should attempt before giving up.
const size_t kNumRetries = 5;

// The delay before we give up on an HTTP request;
const int TIMEOUT = 5 * 1000; // 5 seconds

const uint32 MSG_TIMEOUT = 100;  // must not conflict with BasicPortAllocator.cpp

// Helper routine to remove whitespace from the ends of a string.
void Trim(std::string& str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    str.clear();
    return;
  }

  size_t last = str.find_last_not_of(" \t\r\n");
  ASSERT(last != std::string::npos);
}

// Parses the lines in the result of the HTTP request that are of the form
// 'a=b' and returns them in a map.
typedef std::map<std::string,std::string> StringMap;
void ParseMap(const std::string& string, StringMap& map) {
  size_t start_of_line = 0;
  size_t end_of_line = 0;

  for (;;) { // for each line
    start_of_line = string.find_first_not_of("\r\n", end_of_line);
    if (start_of_line == std::string::npos)
      break;

    end_of_line = string.find_first_of("\r\n", start_of_line);
    if (end_of_line == std::string::npos) {
      end_of_line = string.length();
    }

    size_t equals = string.find('=', start_of_line);
    if ((equals >= end_of_line) || (equals == std::string::npos))
      continue;

    std::string key(string, start_of_line, equals - start_of_line);
    std::string value(string, equals + 1, end_of_line - equals - 1);

    Trim(key);
    Trim(value);

    if ((key.size() > 0) && (value.size() > 0))
      map[key] = value;
  }
}

}

namespace cricket {

// SimplePortAllocator

SimplePortAllocator::SimplePortAllocator(talk_base::NetworkManager* network_manager, const std::string &user_agent)
  : BasicPortAllocator(network_manager), agent_(user_agent) {
  relay_hosts_.push_back("10.211.55.5");
  stun_hosts_.push_back(talk_base::SocketAddress("10.211.55.5",3478));
}

SimplePortAllocator::~SimplePortAllocator() {
}

PortAllocatorSession *SimplePortAllocator::CreateSession(const std::string &name, const std::string &session_type) {
  return new SimplePortAllocatorSession(this, name, session_type, stun_hosts_, relay_hosts_, relay_token_, agent_);
}

// SimplePortAllocatorSession

SimplePortAllocatorSession::SimplePortAllocatorSession(SimplePortAllocator* allocator, const std::string &name, 
						   const std::string &session_type, 
						   const std::vector<talk_base::SocketAddress> &stun_hosts,
						   const std::vector<std::string> &relay_hosts, 
						   const std::string &relay_token, 
						   const std::string &user_agent)
  : BasicPortAllocatorSession(allocator, name, session_type), 
    attempts_(0), relay_hosts_(relay_hosts), stun_hosts_(stun_hosts), relay_token_(relay_token), agent_(user_agent) {
}

void SimplePortAllocatorSession::GetPortConfigurations() {
	StringMap map;

	map["username"] = "test1@bypass";
	map["password"] = "1234";
	map["magic_cookie"] = "1234";

	map["relay.ip"] = "127.0.0.1";
	map["relay.udp_port"] = "8888";

	std::string username = map["username"];
	std::string password = map["password"];
	std::string magic_cookie = map["magic_cookie"];

	std::string relay_ip = map["relay.ip"];
	std::string relay_udp_port = map["relay.udp_port"];
	std::string relay_tcp_port = map["relay.tcp_port"];
	std::string relay_ssltcp_port = map["relay.ssltcp_port"];

	PortConfiguration* config = new PortConfiguration(stun_hosts_[0],
		username,
		password,
		magic_cookie);

	PortConfiguration::PortList ports;
	if (!relay_udp_port.empty()) {
		talk_base::SocketAddress address(relay_ip, atoi(relay_udp_port.c_str()));
		ports.push_back(ProtocolAddress(address, PROTO_UDP));
	}
	if (!relay_tcp_port.empty()) {
		talk_base::SocketAddress address(relay_ip, atoi(relay_tcp_port.c_str()));
		ports.push_back(ProtocolAddress(address, PROTO_TCP));
	}
	if (!relay_ssltcp_port.empty()) {
		talk_base::SocketAddress address(relay_ip, atoi(relay_ssltcp_port.c_str()));
		ports.push_back(ProtocolAddress(address, PROTO_SSLTCP));
	}
	config->AddRelay(ports, 0.0f);
	ConfigReady(config);
}

} // namespace cricket