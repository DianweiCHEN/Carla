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
	__ASSIGNER_TH_POS   = 0,
	__DISPATCHER_TH_POS = 1,
	__WORKER_TH_POS     = 2
};

#define MIN_NUMBER_TH					4
#define MAX_NUMBER_TH					128

/// Initialization function
void TrafficManagerProcessor::Initialize(GlbData *gData)
{
	/// Get total thread count
	gData->threadCount = std::thread::hardware_concurrency();
	gData->threadCount = BOUND(gData->threadCount, MIN_NUMBER_TH, MAX_NUMBER_TH);

	/// PThread array to contain threads
	gData->threadList = new(std::nothrow) std::thread[gData->threadCount];
	if(gData->threadList == nullptr) {
		DEBUG_ASSERT(gData->threadList != nullptr);
	}

	/// Global shared job data
	while(!gData->jobQueue.empty())gData->jobQueue.pop();

	/// Global exit flag to stop processing
	gData->exitFlag = false;

	/// Assigner thread initialization
	gData->threadList[__ASSIGNER_TH_POS] =
			std::thread(TrafficManagerThread::TrafficManagerAssignerThread, gData);

	/// Dispatcher thread initialization
	gData->threadList[__DISPATCHER_TH_POS] =
			std::thread(TrafficManagerThread::TrafficManagerDispatcherThread, gData);

	/// Initialize worker threads
	for (int thCount = __WORKER_TH_POS; thCount < gData->threadCount; thCount++) {

		/// Assign individual threads
		gData->threadList[thCount] =
				std::thread(TrafficManagerThread::TrafficManagerWorkerThread, gData);
	}
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
	std::this_thread::sleep_for(std::chrono::milliseconds(2));

	/// Wait for all threads to exit
	for (int thCount = 0; thCount < gData->threadCount; thCount++) {

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
