// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Metadata.h"

#include <rpc/server.h>
#include <mutex>

namespace carla {
namespace rpc {

  class ::rpc::detail::server_session;

  using key_type = std::intptr_t;

  struct ClientSession {
    ClientSession(key_type _id) : 
      id(_id),
      tick(0),
      time_out(5) {};

    bool operator ==(const ClientSession &_session) const {
      return _session.id == id;
    }

    bool operator ==(key_type _id) const {
      return id == _id;
    }

    key_type id;
    bool tick;
    double time_out;
  };

  class ClientSessions {
  public:
    void Add(key_type id);
    void Remove(key_type id);
    void Tick(key_type id);
    void UntickAll();
    bool AreReady();
    
  private:
    std::vector<ClientSession> _sessions;
    std::mutex _mutex;
  };

} // namespace rpc
} // namespace carla
