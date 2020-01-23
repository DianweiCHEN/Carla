// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/*
 * ----------------- BEGIN LICENSE BLOCK ---------------------------------
 *
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * ----------------- END LICENSE BLOCK -----------------------------------
 */

#pragma once

#include "carla/client/TrafficSign.h"
#include "carla/rpc/TrafficLightState.h"

namespace carla {
namespace client {

  class TrafficLight : public TrafficLandmark {
  public:

    explicit TrafficLight(ActorInitializer init, TrafficLightType type)
    : TrafficLandmark(std::move(init),
    _type(type)) {}

    void SetState(rpc::TrafficLightState state);

    /// Return the current state of the traffic light.
    ///
    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    rpc::TrafficLightState GetState() const;

    void SetGreenTime(float green_time);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetGreenTime() const;

    void SetYellowTime(float yellow_time);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetYellowTime() const;

    void SetRedTime(float red_time);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetRedTime() const;

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetElapsedTime() const;

    void Freeze(bool freeze);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    bool IsFrozen() const;

    uint32_t GetPoleIndex();

    /// Return all traffic lights in the group this one belongs to.
    ///
    /// @note This function calls the simulator
    std::vector<SharedPtr<TrafficLight>> GetGroupTrafficLights();

  private:
    TrafficLightType _type;
  };

} // namespace client
} // namespace carla
