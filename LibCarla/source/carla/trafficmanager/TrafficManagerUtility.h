// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <iostream>

namespace carla {
namespace traffic_manager {

#define ROUND(__x)             (((__x)>0)?((__x)+0.5):((__x)-0.5))
#define BOUND(__x, __y, __z)   (((__x)<(__y))?(__y):(((__x)>(__z))?(__z):(__x)))


class TrafficManagerUtil {
public:

	/// Get IP address of the local system
	static std::pair<std::string, uint16_t> GetLocalIP(const uint16_t sport);

	/// Get current time in mili second
	static unsigned long int GetTimeMiliseconds();
};

} // traffic_manager
} // namespace carla


