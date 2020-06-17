// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Server/CarlaServer.h"

#include <vector>
#include <map>
#include <fstream>
#include <chrono>

#pragma once

std::map<uint64_t, std::vector<uint64_t>> tickTimes;

void SetTickTime(uint64_t frame, int index, uint64_t value)
{
    if (tickTimes.find(frame) != tickTimes.end())
    {
        if (tickTimes[frame].size() <= index)
            tickTimes[frame].resize(index + 1, 0);
        tickTimes[frame][index] = value;
    }
    else
    {
        tickTimes[frame] = std::vector<uint64_t>();
        tickTimes[frame].resize(index + 1, 0);
        tickTimes[frame][index] = value;
    }
}
void TickTimesFlush()
{
    uint64_t prev = 0;
    std::ofstream file("e:\\download\\tickTimes.txt");
    for (auto frame : tickTimes)
    {
        // milli, milli, loops
        if (prev == 0)
            file << frame.first << ": " << frame.second[1] - frame.second[0] << ", " << frame.second[2] << "\n";
        else
            file << frame.first << ": milli " << frame.second[1] - frame.second[0] << ", loops " << frame.second[2] << ", from prev " << frame.second[0] - prev << "\n";
        prev = frame.second[1];
    }
    file.close();
}
