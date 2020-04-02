// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/trafficmanager/TrafficManagerGlobalData.h"

namespace carla {
namespace traffic_manager {

/// The function of this class is to process the registered vehicles
class TrafficManagerProcessor {

public:

	/// To initialize/allocate the traffic manager.
	void Initialize(GlbData *gData);

	/// To release the traffic manager.
	void Release(GlbData *gData);

	/// Constructor
	TrafficManagerProcessor() {};

	/// Destructor.
	~TrafficManagerProcessor() {};
};

} // namespace traffic_manager
} // namespace carla


