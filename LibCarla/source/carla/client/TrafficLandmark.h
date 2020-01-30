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

#include "carla/client/Actor.h"
#include "carla/geom/Vector3D.h"

// TODO (doterop): is namespace right?

namespace carla {
namespace client {

  class TrafficLandmark : public Actor {
  public:

    enum class LandmarkType : int32_t {
      Invalid = -1,
      Unknown = 0,
      TrafficSign, // Traffic Sign (E.G. Yield, Stop).
      TrafficLight,
      Pole,
      GuidePost,
      Tree,
      StreetLamp,
      Postbox,
      Manhole,
      Powercabinet,
      FireHydrant,
      Bollard,
      Other
    };

    explicit TrafficLandmark(ActorInitializer init, LandmarkType type)
      : Actor(std::move(init)),
      _type(type) {}

    const geom::BoundingBox &GetTriggerVolume() const {
      return ActorState::GetBoundingBox();
    }

  protected:
    LandmarkType _type;
    int32_t _traffic_landmark_id;

  };

} // namespace client
} // namespace carla
