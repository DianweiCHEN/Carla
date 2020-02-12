// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/LocalizationUtils.h"

#include <iterator>

namespace carla {
namespace traffic_manager {

namespace LocalizationConstants {
  const static float GRID_SIZE = 2.0f;
} // namespace LocalizationConstants

  using namespace LocalizationConstants;

  float DeviationCrossProduct(Actor actor, const cg::Location &vehicle_location, const cg::Location &target_location) {

    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0.0f;
    heading_vector = heading_vector.MakeUnitVector();
    cg::Location next_vector = target_location - vehicle_location;
    next_vector.z = 0.0f;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      const float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
      return cross_z;
    } else {
      return 0.0f;
    }
  }

  float DeviationDotProduct(Actor actor, const cg::Location &vehicle_location, const cg::Location &target_location, bool rear_offset) {

    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0.0f;
    heading_vector = heading_vector.MakeUnitVector();
    cg::Location next_vector;

    if (!rear_offset) {
      next_vector = target_location - vehicle_location;
    } else {
      const auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor);
      const float vehicle_half_length = vehicle_ptr->GetBoundingBox().extent.x;
      next_vector = target_location - (cg::Location(-1* vehicle_half_length * heading_vector)
                                        + vehicle_location);
    }

    next_vector.z = 0.0f;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      const float dot_product = cg::Math::Dot(next_vector, heading_vector);
      return dot_product;
    } else {
      return 0.0f;
    }
  }

  std::array<geom::Vector2D, 2> GetWorldVerticesAABB(const geom::BoundingBox& bbox, const geom::Transform& transform) {
    auto bbox_vertices = bbox.GetWorldVertices(transform);
    float min_x = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::lowest();
    float max_x = std::numeric_limits<float>::lowest();
    float min_y = std::numeric_limits<float>::max();
    for (const geom::Location& vertex : bbox_vertices) {
      min_x = (vertex.x < min_x) ? vertex.x : min_x;
      max_y = (vertex.y > max_y) ? vertex.y : max_y;
      max_x = (vertex.x > max_x) ? vertex.x : max_x;
      min_y = (vertex.y < min_y) ? vertex.y : min_y;
    }

    return {{
      geom::Vector2D(min_x, max_y),
      geom::Vector2D(max_x, min_y)
    }};
  }

  std::pair<int64_t, int64_t> TrackTraffic::GetSpatialKey(geom::Vector2D pos) const {
    return std::make_pair(
        static_cast<int64_t>(pos.x / GRID_SIZE),
        static_cast<int64_t>(pos.y / GRID_SIZE)
    );
  }

  void TrackTraffic::UpdateActor(const Actor& actor) {
    CleanActor(actor->GetId());

    // Computing AABB based on vehicle bounding box.
    const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    geom::BoundingBox bbox = vehicle->GetBoundingBox();
    std::array<geom::Vector2D, 2> aabb = GetWorldVerticesAABB(bbox, vehicle->GetTransform());
    _mapped_aabb.insert({actor->GetId(), aabb});

    // Adding information to spatial hashing.
    auto top_left = GetSpatialKey(aabb[0]);
    auto bottom_right = GetSpatialKey(aabb[1]);
    for (int64_t x = top_left.first; x <= bottom_right.first; x++) {
      for (int64_t y = top_left.second; y >= bottom_right.second; y--) {
        _spatial_hash[std::make_pair(x, y)].insert(actor->GetId());
      }
    }
  }

  void TrackTraffic::UpdateActor(const Actor& actor, const Buffer& buffer) {
    CleanActor(actor->GetId());

    // Computing AABB path boundary based on waypoint buffer.
    const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    geom::BoundingBox bbox = vehicle->GetBoundingBox();
    std::array<geom::Vector2D, 2> aabb = GetWorldVerticesAABB(bbox, vehicle->GetTransform());

    float& min_x = aabb[0].x;
    float& max_y = aabb[0].y;
    float& max_x = aabb[1].x;
    float& min_y = aabb[1].y;
    for (const auto& swp : buffer) {
      geom::Transform transform = swp->GetWaypoint()->GetTransform();
      std::array<geom::Location, 8> vertices = bbox.GetWorldVertices(transform);

      for (const geom::Location& vertex : vertices) {
        min_x = (vertex.x < min_x) ? vertex.x : min_x;
        max_y = (vertex.y > max_y) ? vertex.y : max_y;
        max_x = (vertex.x > max_x) ? vertex.x : max_x;
        min_y = (vertex.y < min_y) ? vertex.y : min_y;
      }
    }

    _mapped_aabb.insert({actor->GetId(), aabb});

    // Adding information to spatial hashing.
    auto top_left = GetSpatialKey(aabb[0]);
    auto bottom_right = GetSpatialKey(aabb[1]);
    for (int64_t x = top_left.first; x <= bottom_right.first; x++) {
      for (int64_t y = top_left.second; y >= bottom_right.second; y--) {
        _spatial_hash[std::make_pair(x, y)].insert(actor->GetId());
      }
    }
  }

  bool TrackTraffic::CleanActor(ActorId actor_id) {
    // Cleaning information spatial hashing.
    if (_mapped_aabb.count(actor_id)) {
      const auto& aabb = _mapped_aabb.at(actor_id);
      auto top_left = GetSpatialKey(aabb[0]);
      auto bottom_right = GetSpatialKey(aabb[1]);
    for (int64_t x = top_left.first; x <= bottom_right.first; x++) {
      for (int64_t y = top_left.second; y >= bottom_right.second; y--) {
          _spatial_hash[std::make_pair(x, y)].erase(actor_id);

          if (_spatial_hash[std::make_pair(x, y)].empty()) {
            _spatial_hash.erase(std::make_pair(x, y));
          }
        }
      }

      // Cleaning mapped AABB.
      _mapped_aabb.erase(actor_id);
      return true;
    }
    return false;
  }

  std::array<geom::Vector2D, 2> TrackTraffic::GetActorAABB(ActorId actor_id) const {
    return _mapped_aabb.at(actor_id);
  }

  ActorIdSet TrackTraffic::GetOverlappingActors(ActorId actor_id) const {
    ActorIdSet result;
    if (!_mapped_aabb.count(actor_id)) {
      return result;
    }

    const auto& aabb = _mapped_aabb.at(actor_id);
    auto top_left = GetSpatialKey(aabb[0]);
    auto bottom_right = GetSpatialKey(aabb[1]);
    for (int64_t x = top_left.first; x <= bottom_right.first; x++) {
      for (int64_t y = top_left.second; y >= bottom_right.second; y--) {
        auto actor_ids = _spatial_hash.at(std::make_pair(x, y));
        result.insert(actor_ids.begin(), actor_ids.end());
      }
    }
    return result;
  }

  std::unordered_set<OverlappingPair> TrackTraffic::GetOverlappingActors() const {
    std::unordered_set<OverlappingPair> result;
    for (auto& cell : _spatial_hash) {
      auto& actors = cell.second;

      if (actors.size() >= 2u) {
        for (auto i = actors.begin(); i != actors.end(); i++) {
          for (auto j = std::next(i); j != actors.end(); j++) {
            result.insert({{*i, *j}});
          }
        }
      }
    }
    return result;
  }

  void TrackTraffic::UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {

      ActorIdSet& actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
      if (actor_id_set.find(actor_id) == actor_id_set.end()) {
        actor_id_set.insert(actor_id);
      }
    } else {

      waypoint_overlap_tracker.insert({waypoint_id, {actor_id}});
    }
  }

  void TrackTraffic::RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {

      auto& actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
      if (actor_id_set.find(actor_id) != actor_id_set.end()) {
        actor_id_set.erase(actor_id);
      }

      if (actor_id_set.size() == 0) {
        waypoint_overlap_tracker.erase(waypoint_id);
      }
    }
  }

  ActorIdSet TrackTraffic::GetPassingVehicles(uint64_t waypoint_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
      return waypoint_overlap_tracker.at(waypoint_id);
    } else {
      return ActorIdSet();
    }

  }

} // namespace traffic_manager
} // namespace carla
