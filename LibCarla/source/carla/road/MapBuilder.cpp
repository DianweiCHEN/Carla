// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoVisitor.h"

using namespace carla::road::element;

namespace carla {
namespace road {

  SharedPtr<Map> MapBuilder::Build() {

    SetTotalRoadSegmentLength();

    CreatePointersBetweenRoadSegments();

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return SharedPtr<Map>(new Map{std::move(_map_data)});
  }

  void MapBuilder::CreateLaneAccess(int road_id, int lane_id, double s, std::string restriction)
  {
    auto access = SharedPtr<RoadInfoLaneAccess>(new RoadInfoLaneAccess(s, restriction));
    // TODO: get lane with lane_id and add to list in lane. Same for below functions besides road elevation
  }

  void MapBuilder::CreateLaneBorder(int road_id, int lane_id, double s, double a, double b, double c, double d)
  {
    auto border = SharedPtr<RoadInfoLaneBorder>(new RoadInfoLaneBorder(s, a, b, c, d));
  }

  void MapBuilder::CreateLaneHeight(int road_id, int lane_id, double s, double inner, double outer)
  {
    auto height = SharedPtr<RoadInfoLaneHeight>(new RoadInfoLaneHeight(s, inner, outer));
  }

  void MapBuilder::CreateLaneMaterial(int road_id, int lane_id, double s, std::string surface, double friction, double roughness)
  {
    auto material = SharedPtr<RoadInfoLaneMaterial>(new RoadInfoLaneMaterial(s, surface, friction, roughness));
  }

  void MapBuilder::CreateLaneOffset(int road_id, int lane_id, double s, double a, double b, double c, double d)
  {
    auto offset = SharedPtr<RoadInfoLaneOffset>(new RoadInfoLaneOffset(s, a, b, c, d));
  }

  void MapBuilder::CreateLaneRule(int road_id, int lane_id, double s, std::string value)
  {
    auto rule = SharedPtr<RoadInfoLaneRule>(new RoadInfoLaneRule(s, value));
  }

  void MapBuilder::CreateLaneVisibility(int road_id, int lane_id, double s, double forward, double back, double left, double right)
  {
    auto visibility = SharedPtr<RoadInfoLaneVisibility>(new RoadInfoLaneVisibility(s, forward, back, left, right));
  }

  void MapBuilder::CreateLaneWidth(int road_id, int lane_id, double s, double a, double b, double c, double d)
  {
    auto width = SharedPtr<RoadInfoLaneWidth>(new RoadInfoLaneWidth(s, a, b, c, d));
  }

  void MapBuilder::CreateLaneMark(
      int road_id,
      int lane_id,
      double s,
      std::string type,
      std::string weight,
      std::string color,
      std::string material,
      double width,
      std::string lane_change,
      double height)
  {
    RoadInfoMarkRecord::LaneChange lc;
    using tl = std::tolower;
    if (tl(lane_change) == "increase")
    {
      lc = RoadInfoMarkRecord::LaneChange::Increase;
    }
    else if (tl(lane_change) == "decrease")
    {
      lc = RoadInfoMarkRecord::LaneChange::Decrease;
    }
    else if (tl(lane_change) == "both")
    {
      lc = RoadInfoMarkRecord::LaneChange::Both;
    }
    else
    {
      lc = RoadInfoMarkRecord::LaneChange::None;
    }
    auto mark = SharedPtr<RoadInfoMarkRecord>(new RoadInfoMarkRecord(s, type, weight, color, material, width, lc, height));
  }

  void MapBuilder::CreateLaneSpeed(int road_id, int lane_id, double s, double max, std::string unit)
  {
    auto speed = SharedPtr<RoadInfoVelocity>(new RoadInfoVelocity(s, max));
  }

  void MapBuilder::CreateRoadElevationInfo(int road_id, int road_id, double s, double a, double b, double c, double d)
  {
    auto elevation = SharedPtr<RoadElevationInfo>(new RoadElevationInfo(s, a, b, c, d));
  }

} // namespace road
} // namespace carla
