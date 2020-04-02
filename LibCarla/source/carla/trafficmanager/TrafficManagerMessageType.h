// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <stdio.h>
#include <string>

namespace carla {
namespace traffic_manager {

/// Various processing type
enum _msg_type_info_ {
	VEHILE_PROCESS = 1,
};

/// Message data keeps captured data from different source
typedef struct __message_data__ {

	/// Type of the message (Mainly processing type)
	unsigned long int msgType;

	/// Time-stamp of the captured data
	unsigned long int timestamp;

	/// Custom parameter list for each message (allocated size PARAM_LIST_SIZE)
	std::string msgInfo;

} MsgData;

} // namespace traffic_manager
} // namespace carla
