// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "carla/Debug.h"
#include "carla/trafficmanager/TrafficManagerProcessor.h"
#include "carla/trafficmanager/TrafficManagerGlobalData.h"
#include "carla/trafficmanager/TrafficManagerThread.h"
#include "carla/trafficmanager/TrafficManagerUtility.h"

namespace carla {
namespace traffic_manager {

enum _thread_name {
	__ASSIGNER_TH   = 0,
	__DISPATCHER_TH = 1
};

/// Initialization function
void TrafficManagerProcessor::Initialize(GlbData *gData)
{
	/// PThread array to contain threads
	gData->threadList = new(std::nothrow) std::thread[NUMBER_OF_WORKER_TH + 2];
	if(gData->threadList == nullptr) {
		DEBUG_ASSERT(gData->threadList != nullptr);
	}

	/// Global shared job data
	while(!gData->jobQueue.empty())gData->jobQueue.pop();

	/// Global exit flag to stop processing
	gData->exitFlag = false;

	/// Initialize worker threads
	for (int thCount = 0; thCount < NUMBER_OF_WORKER_TH; thCount++) {

		/// Assign individual threads
		gData->threadList[thCount] =
				std::thread(TrafficManagerThread::TrafficManagerWorkerThread, gData);
	}

	/// Assigner thread initialization
	gData->threadList[NUMBER_OF_WORKER_TH + __ASSIGNER_TH] =
			std::thread(TrafficManagerThread::TrafficManagerAssignerThread, gData);

	/// Dispatcher thread initialization
	gData->threadList[NUMBER_OF_WORKER_TH + __DISPATCHER_TH] =
			std::thread(TrafficManagerThread::TrafficManagerDispatcherThread, gData);
}

/// Release function
void TrafficManagerProcessor::Release(GlbData *gData)
{
	/// Get lock to set exit flag
	gData->dlock.lock();

	/// Set exit flag
	gData->exitFlag = true;

	/// Global shared job data
	while(!gData->jobQueue.empty())gData->jobQueue.pop();

	/// Unlock mutex for other threads
	gData->dlock.unlock();

	/// Notify all threads to exit
	gData->cond.notify_all();

	/// Wait for all threads to exit
	std::this_thread::sleep_for(std::chrono::milliseconds(5));

	/// Wait for all threads to exit
	for (int thCount = 0; thCount < NUMBER_OF_WORKER_TH + 2; thCount++) {

		/// release individual threads
		if(gData->threadList[thCount].joinable()) {
			gData->threadList[thCount].join();
		}
	}

	/// Clear thread list
	delete [] gData->threadList;
}


} // namespace traffic_manager
} // namespace carla
