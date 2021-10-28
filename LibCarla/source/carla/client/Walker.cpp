// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Walker.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  void Walker::ApplyControl(const Control &control) {
    if (control != _control) {
      GetEpisode().Lock()->ApplyControlToWalker(*this, control);
      _control = control;
    }
  }

  void Walker::ApplyControl(const BoneControl &bone_control) {
    GetEpisode().Lock()->ApplyBoneControlToWalker(*this, bone_control);
  }

  Walker::Control Walker::GetWalkerControl() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.walker_control;
  }

  // Walker::Control Walker::GetBonesTransform() const {
  Walker::BoneControl Walker::GetBonesTransform() {
    return GetEpisode().Lock()->GetBonesTransform(*this);
  }
} // namespace client
} // namespace carla
