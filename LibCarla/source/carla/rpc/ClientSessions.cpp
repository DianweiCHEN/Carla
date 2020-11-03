// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/rpc/ClientSessions.h"

namespace carla {
namespace rpc {

void ClientSessions::Add(key_type id) {
  std::lock_guard<std::mutex> lock(_mutex);
  _sessions.emplace_back(id);
  log_info("Client ", id, " connected: ", _sessions.size());
}

void ClientSessions::Remove(key_type id) {
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = std::find(_sessions.begin(), _sessions.end(), id);
  if (it != _sessions.end()) {
    _sessions.erase(it);
    log_info("Client ", id, " disconnected: ", _sessions.size());
  }
}

void ClientSessions::Tick(key_type id) {
  log_info("Client searching for ", id);
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = std::find(_sessions.begin(), _sessions.end(), id);
  if (it != _sessions.end()) {
    it->tick = true;
    log_info("Client ", id, " ticks");
  }
}

void ClientSessions::UntickAll() {
  std::lock_guard<std::mutex> lock(_mutex);
  for (auto &session : _sessions)
    session.tick = false;
  log_info("Untick all clients: ", _sessions.size());
}

bool ClientSessions::AreReady() {
  std::lock_guard<std::mutex> lock(_mutex);
  bool res = false;
  for (auto &session : _sessions)
    res &= session.tick;
  log_info("Clients ready: ", res);
  return res;
}

} // namespace rpc
} // namespace carla
