// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <chrono>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "carla/trafficmanager/TrafficManagerGlobalData.h"
#include "carla/trafficmanager/TrafficManagerUtility.h"
#include "carla/trafficmanager/TrafficManagerThread.h"
#include "carla/trafficmanager/TrafficManagerMessageType.h"

#define DEBUG_PRINT_THREAD		1

namespace carla {
namespace traffic_manager {

void TrafficManagerThread::TrafficManagerAssignerThread(GlbData *glbdata)
{
	/// Run for ever
	while(true) {

		/// Contains of message should be allocated by assigner thread and
		/// Free by worker thread which consumes the data
		MsgData mData;

		/// Allocate message data
		mData.msgType   = VEHILE_PROCESS;

		/// Capture function  - get vehicle information from simulator
		/// TODO:

		/// Time-stamp function
		mData.timestamp = TrafficManagerUtil::GetTimeMiliseconds();

		/// Library call (pre-processing)
		/// TODO:

		// Acquire the lock for assign new message to the queue
		glbdata->dlock.lock();

		/// If global exit flag set stop processing and exit
		if(glbdata->exitFlag == true) {
			glbdata->dlock.unlock();
			break;
		}

		/// DEBUG Run in 30-40fps
		std::this_thread::sleep_for(std::chrono::milliseconds(25));

		/// Add all Vehicles for processing
		for (int var = 0; var < 100; ++var) {

			/// Set vehicle information
			/// TODO:
			mData.msgInfo = std::to_string(var);

			/// Check message properly inserted or not
			glbdata->jobQueue.push(mData);
		}

		/// DEBUG current job queue size
		std::cout << "TrafficManagerAssignerThread assigned ... at " << mData.timestamp << " : " << glbdata->jobQueue.size() << std::endl;

		/// Release data lock
		glbdata->dlock.unlock();

		/// Signal worker thread that message present
		glbdata->cond.notify_all();
	}

#if DEBUG_PRINT_THREAD
	/// Exit of thread
	std::cout << "TrafficManagerAssignerThread exited ... at " << TrafficManagerUtil::GetTimeMiliseconds() << std::endl;
#endif
}


void TrafficManagerThread::TrafficManagerWorkerThread(GlbData *glbdata)
{
	/// Set default
	bool threadExitFlag;

	/// Run for ever
	while(true) {

		/// Set default
		threadExitFlag = false;

		/// Wait for signal that message present in queue
		std::unique_lock<std::mutex> signalLock(glbdata->slock);
		glbdata->cond.wait(signalLock);

		/// Free signal lock so that other workers also can act on the signal (if present)
		signalLock.unlock();

#if DEBUG_PRINT_THREAD
		/// Worker thread debug print
		std::cout << "TrafficManagerWorkerThread Got Signal ... at " << TrafficManagerUtil::GetTimeMiliseconds() << "ID : " << std::this_thread::get_id() << std::endl;
#endif

		/// As long as data is present in the queue process them
		while(true) {

			/// Get data lock to read new message from the queue
			glbdata->dlock.lock();

			/// If global exit flag set
			if(glbdata->exitFlag == true) {

				/// Unlock mutex for other threads
				glbdata->dlock.unlock();

				/// Mark thread exit flag present
				threadExitFlag = true;
				break;
			}

			/// If message preset in the queue
			if(!glbdata->jobQueue.empty()) {

				/// Contains of message already allocated by assigner thread and
				/// It going to be freed by worker thread after consumption of the data
				MsgData mData = glbdata->jobQueue.front();

				/// Remove job from the list as job is already taken by the thread
				glbdata->jobQueue.pop();

#if DEBUG_PRINT_THREAD
				std::cout << "TrafficManagerWorkerThread Got Job " << mData.msgType << " of " << mData.timestamp << " taken ... at " << TrafficManagerUtil::GetTimeMiliseconds() << std::endl;
#endif
				/// Unlock mutex for other threads before consuming data
				/// As it may take time to do so.
				glbdata->dlock.unlock();

				/// Consume / Process data
				/// TODO:
			} else {

				/// Unlock mutex for other threads
				glbdata->dlock.unlock();

				/// As message queue is empty
				break;
			}
		}

		/// If exit flag is set, then stop processing
		if(threadExitFlag) break;
	}

#if DEBUG_PRINT_THREAD
	/// Exit of thread
	std::cout << "TrafficManagerThread::TrafficManagerWorkerThread exited ... at " << TrafficManagerUtil::GetTimeMiliseconds() << std::endl;
#endif
}



void TrafficManagerThread::TrafficManagerDispatcherThread(GlbData *glbdata)
{
	/// Run for ever
	while(true) {

		/// Get data lock to read new message from the queue
		glbdata->dlock.lock();

		/// If global exit flag set
		if(glbdata->exitFlag == true) {

			/// Unlock mutex for other threads
			glbdata->dlock.unlock();
			break;
		}

		/// If message preset in the queue
		if(!glbdata->vehicleInfo.empty()) {

			/// Prepare command to dispatch
			/// Add all Vehicles for processing
			for (int var = 0; var < 1000; ++var) {

				/// Set vehicle information
				/// TODO:
			}

			/// Send data to simulator
			/// TODO:
		}
		/// Unlock mutex for other threads before consuming data
		/// As it may take time to do so.
		glbdata->dlock.unlock();

		/// Run in 30-40fps
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

#if DEBUG_PRINT_THREAD
	/// Exit of thread
	std::cout << "TrafficManagerDispatcherThread exited ... at " << TrafficManagerUtil::GetTimeMiliseconds() << std::endl;
#endif
}

} // namespace traffic_manager
} // namespace carla
