// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Actor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"
#include "carla/profiler/Tracer.h"

namespace carla {
namespace client {

  geom::Location Actor::GetLocation() const {
    TRACE_SCOPE_FUNCTION("Actor");
    return GetEpisode().Lock()->GetActorLocation(*this);
  }

  geom::Transform Actor::GetTransform() const {
    TRACE_SCOPE_FUNCTION("Actor");
    return GetEpisode().Lock()->GetActorTransform(*this);
  }

  geom::Vector3D Actor::GetVelocity() const {
    TRACE_SCOPE_FUNCTION("Actor");
    return GetEpisode().Lock()->GetActorVelocity(*this);
  }

  geom::Vector3D Actor::GetAngularVelocity() const {
    TRACE_SCOPE_FUNCTION("Actor");
    return GetEpisode().Lock()->GetActorAngularVelocity(*this);
  }

  void Actor::SetAngularVelocity(const geom::Vector3D &vector) {
    TRACE_SCOPE_FUNCTION("Actor");
    GetEpisode().Lock()->SetActorAngularVelocity(*this, vector);
  }

  geom::Vector3D Actor::GetAcceleration() const {
    return GetEpisode().Lock()->GetActorAcceleration(*this);
  }

  void Actor::SetLocation(const geom::Location &location) {
    TRACE_SCOPE_FUNCTION("Actor");
    GetEpisode().Lock()->SetActorLocation(*this, location);
  }

  void Actor::SetTransform(const geom::Transform &transform) {
    TRACE_SCOPE_FUNCTION("Actor");
    GetEpisode().Lock()->SetActorTransform(*this, transform);
  }

  void Actor::SetVelocity(const geom::Vector3D &vector) {
    TRACE_SCOPE_FUNCTION("Actor");
    GetEpisode().Lock()->SetActorVelocity(*this, vector);
  }

  void Actor::AddImpulse(const geom::Vector3D &vector) {
    TRACE_SCOPE_FUNCTION("Actor");
    GetEpisode().Lock()->AddActorImpulse(*this, vector);
  }

  void Actor::SetSimulatePhysics(const bool enabled) {
    TRACE_SCOPE_FUNCTION("Actor");
    GetEpisode().Lock()->SetActorSimulatePhysics(*this, enabled);
  }

  bool Actor::Destroy() {
    TRACE_SCOPE_FUNCTION("Actor");
    if (IsAlive()) {
      // Let the exceptions leave the function, IsAlive() will still be true.
      _is_alive = !GetEpisode().Lock()->DestroyActor(*this);
    } else {
      log_warning(
          "attempting to destroy an actor that is already dead:",
          GetDisplayId());
      _is_alive = false;
    }
    return _is_alive;
  }

} // namespace client
} // namespace carla
