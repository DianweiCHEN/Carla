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

//#include "carla/client/TrafficLandmark.h"
#include "carla/client/Actor.h"

namespace carla {
namespace client {

  /* class TrafficSign : public Actor {
  public:
    enum class TrafficSignType : int32_t {
      Invalid = 0,
    };

    explicit TrafficSign(ActorInitializer init, TrafficSignType type = TrafficSignType::Invalid)
      : TrafficLandmark(std::move(init), TrafficLandmark::LandmarkType::TrafficSign),
      _sign_type(type) {}

    TrafficSignType GetType() const { return _sign_type; }

  private:
    TrafficSignType _sign_type;
  }; */

  class TrafficSign : public Actor {
  public:

    explicit TrafficSign(ActorInitializer init) : Actor(std::move(init)) {}

    const geom::BoundingBox &GetTriggerVolume() const {
      return ActorState::GetBoundingBox();
    }
  };

} // namespace client
} // namespace carla
