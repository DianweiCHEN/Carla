// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <thread>
#include "carla/trafficmanager/TrafficManagerGlobalData.h"

namespace carla {
namespace traffic_manager {

/// The function of this class is to assign vehicles to job queue
class TrafficManagerThread {

private:
	/// Constructor
	TrafficManagerThread() {};

	/// Destructor.
	~TrafficManagerThread() {};

public:

	/// Assigner thread function
	static void TrafficManagerAssignerThread(GlbData *glbdata);

	/// Worker thread function
	static void TrafficManagerWorkerThread(GlbData *glbdata);

	/// Dispatcher/Sender thread function
	static void TrafficManagerDispatcherThread(GlbData *glbdata);

};

} // namespace traffic_manager
} // namespace carla

