// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include <map>
#include <queue>

#include "carla/trafficmanager/TrafficManagerMessageType.h"

namespace carla {
namespace traffic_manager {

#define NUMBER_OF_WORKER_TH			8 /// +2 for Assigner & Dispatcher thread

/// Global data to contain System information
typedef struct __global_data__ {

	/// Thread array to contain threads
	std::thread *threadList;

	/// Declaration of thread condition variable for common signal
  std::condition_variable cond;

	// Declaration of thread 'condition variable' lock
	std::mutex slock;

	/// Declaration of thread 'shared data' lock
	std::mutex dlock;


	/// Global shared job message data
	std::queue <MsgData> jobQueue;

	/// Global shared vehicle information
	std::map <std::string, int> vehicleInfo;

	/// Global exit flag to stop processing
	bool exitFlag;

} GlbData;

} // namespace traffic_manager
} // namespace carla
