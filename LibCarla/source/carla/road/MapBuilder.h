// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/road/Map.h"

#include <map>
#include <string>

namespace carla {
namespace road {

  class MapBuilder {
  public:

    SharedPtr<Map> Build();

    void CreateLaneAccess(int road_id, int lane_id, double s, std::string restriction);

    void CreateLaneBorder(int road_id, int lane_id, double s, double a, double b, double c, double d);

    void CreateLaneHeight(int road_id, int lane_id, double s, double inner, double outer);

    void CreateLaneMaterial(int road_id, int lane_id, double s, std::string surface, double friction, double roughness);

    void CreateLaneOffset(int road_id, int lane_id, double s, double a, double b, double c, double d);

    void CreateLaneRule(int road_id, int lane_id, double s, std::string value);

    void CreateLaneVisibility(int road_id, int lane_id, double s, double forward, double back, double left, double right);

    void CreateLaneWidth(int road_id, int lane_id, double s, double a, double b, double c, double d);

    void CreateLaneMark(
        int road_id,
        int lane_id,
        double s,
        std::string type,
        std::string weight,
        std::string color,
        std::string material,
        double width,
        std::string lane_change,
        double height);

    void CreateLaneSpeed(int road_id, int lane_id, double s, double max, std::string unit);

    void CreateRoadElevationInfo(int road_id, double s, double a, double b, double c, double d);

  private:

    /// Set the total length of each road based on the geometries
    void SetTotalRoadSegmentLength();

    /// Create the pointers between RoadSegments based on the ids
    void CreatePointersBetweenRoadSegments();

    MapData _map_data;

  };

} // namespace road
} // namespace carla
