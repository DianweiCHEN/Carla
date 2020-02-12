// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"
#include "carla/geom/Location.h"
#include "carla/road/RoadTypes.h"
#include "carla/rpc/ActorId.h"

#include "carla/geom/BoundingBox.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector2D.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

#include <array>
#include <boost/unordered_map.hpp>

#include <boost/functional/hash.hpp>

namespace carla {
namespace traffic_manager {

  struct OverlappingPair {

    carla::ActorId first;
    carla::ActorId second;

    bool operator==(const OverlappingPair &other) const {
      return ((first == other.first && second == other.second) ||
      (second == other.first && first == other.second));
    }

  };

}  // namespace traffic_manager
}  // namespace carla

namespace std {

  template <>
  struct hash<carla::traffic_manager::OverlappingPair> {
    std::size_t operator()(const carla::traffic_manager::OverlappingPair &pair) const {
      std::size_t seed = 0u;
      boost::hash_combine(seed, boost::hash_value(pair.first));
      boost::hash_combine(seed, boost::hash_value(pair.second));
      return seed;
    }
  };

}  // namespace std

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  namespace cg = carla::geom;

  using Actor = carla::SharedPtr<cc::Actor>;
  using ActorId = carla::ActorId;
  using ActorIdSet = std::unordered_set<ActorId>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using Buffer = std::deque<SimpleWaypointPtr>;
  using GeoGridId = carla::road::JuncId;

  class TrackTraffic{

  private:

    std::unordered_map<ActorId, std::array<geom::Vector2D, 2>> _mapped_aabb;
    boost::unordered_map<std::pair<int64_t, int64_t>, std::unordered_set<ActorId>> _spatial_hash;

    /// Structure to keep track of overlapping waypoints between vehicles.
    using WaypointOverlap = std::unordered_map<uint64_t, ActorIdSet>;
    WaypointOverlap waypoint_overlap_tracker;

  public:

    TrackTraffic() = default;
    ~TrackTraffic() = default;

    void UpdateActor(const Actor &actor);
    void UpdateActor(const Actor &actor, const Buffer &buffer);

    bool CleanActor(ActorId actor_id);

    std::array<geom::Vector2D, 2> GetActorAABB(ActorId actor_id) const;  // Remove

    ActorIdSet GetOverlappingActors(ActorId actor_id) const;
    std::unordered_set<OverlappingPair> GetOverlappingActors() const;

    /// Methods to update, remove and retrieve vehicles passing through a
    /// waypoint.
    void UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    void RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    ActorIdSet GetPassingVehicles(uint64_t waypoint_id);

  private:

    std::pair<int64_t, int64_t> GetSpatialKey(geom::Vector2D pos) const;
  };

  /// Returns the cross product (z component value) between the vehicle's
  /// heading vector and the vector along the direction to the next
  /// target waypoint on the horizon.
  float DeviationCrossProduct(Actor actor, const cg::Location &vehicle_location,
  const cg::Location &target_location);
  /// Returns the dot product between the vehicle's heading vector and
  /// the vector along the direction to the next target waypoint on the horizon.
  float DeviationDotProduct(Actor actor, const cg::Location &vehicle_location,
  const cg::Location &target_location, bool rear_offset = false);

  /// Returns the position of 2 vertices (top-left corner and bottom-right
  /// corner) in world space.
  std::array<geom::Vector2D, 2> GetWorldVerticesAABB(const geom::BoundingBox &bbox,
  const geom::Transform &transform);

} // namespace traffic_manager
} // namespace carla
