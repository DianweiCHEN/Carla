// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/GPULidarSerializer.h"

#include "carla/geom/Math.h"
#include "carla/geom/Transform.h"
#include "carla/image/ImageView.h"
#include "carla/sensor/data/LidarMeasurement.h"

#include <string>

#include <cmath>

namespace carla {
namespace sensor {
namespace s11n {

  /// Helper function to fill a buffer
  template <typename T>
  static size_t AddToBuffer(void *it, const T *data) {
    std::memcpy(it, data, sizeof(T));
    return sizeof(T);
  }

  static geom::Location convert_2d_to_3d(
      const float fov,
      const float far,
      const float width,
      const float height,
      const float x,
      const float y,
      const float norm_depth) {

    // Intrinsic Matrix            |  Inverse Of K
    //        {{   f, 0.0, w/2 },  |         {{ 1/f, 0.0, -(w/2)/f },
    // K =     { 0.0,   f, h/2 },  |  K^-1 =  { 0.0, 1/f, -(h/2)/f },
    //         { 0.0, 0.0, 1.0 }}  |          { 0.0, 0.0,      1.0 }}

    const float f = width / (2.0f * std::tan(
        fov * carla::geom::Math::pi() / 360.0f));

    //                   {{   x },   {{ (x-(w/2))/f },
    // pixel_3d = K^-1 ·  {   y }, =  { (y-(h/2))/f },
    //                    { 1.0 }}    {         1.0 }}

    const geom::Location pixel_3d(
        (x - (width  / 2.f)) / f,
        (y - (height / 2.f)) / f,
        1.0f);

    // 3d coord = pixel 3d * max render distance * normalized depth
    return pixel_3d * far * norm_depth;
  }

  static void screen_to_sensor_coordinates(geom::Location &loc) {
    // screen to sensor local coordinates:
    //     z               z
    //    /                |  x
    //   /______ x   -->   | /
    //   |                 |/_____ y
    //   |
    //   y

    loc.x = -loc.x;
    geom::Transform(
        geom::Location(0.0f, 0.0f, 0.0f),
        geom::Rotation(
        0.0f,
        90.0f,
        90.0f)).TransformPoint(loc);
  }

  SharedPtr<SensorData> GPULidarSerializer::Deserialize(RawData data) {
    auto sensor_header = data.GetHeader();
    using SensorHeaderType = std::remove_reference_t<decltype(sensor_header)>;
    const auto &lidar_header = DeserializeHeader(data);

    // depth image size attributes
    const float fov = lidar_header.fov;
    const uint32_t width = lidar_header.max_horizontal_points;
    const uint32_t height = lidar_header.channels;

    // other lidar attributes
    const float horizontal_angle = lidar_header.horizontal_angle;

    // total points to convert this frame
    // const uint32_t frame_width = lidar_header.current_horizontal_points;

    // while we are rotating the sensor itself, we must rectify the rotation to
    // keep the transform straight as it should be
    sensor_header.sensor_transform.rotation.yaw -= horizontal_angle;

    // create a buffer with the same format as LidarMeasurement does.
    // define the buffer size
    Buffer buf(
        sizeof(SensorHeaderType) +
        sizeof(float) + // horizontal angle
        sizeof(uint32_t) + // channel count
        sizeof(uint32_t) * height + // number of points per channel
        sizeof(geom::Location) * width * height); // points

    // the horizontal angle that client have, must be the angle
    // in the latest column of points
    const auto corrected_horizontal_angle = horizontal_angle + fov / 2.0f;

    // copy data to the buffer that will be used by the client
    auto it = buf.begin();
    it += AddToBuffer<SensorHeaderType>(it, &sensor_header); // header
    it += AddToBuffer<float>(it, &corrected_horizontal_angle); // horizontal angle
    it += AddToBuffer<uint32_t>(it, &height); // num of channels

    // for channel in channels
    for (uint32_t i = 0; i < height; ++i) {
      // num of points per channel (in our case, is always
      // current_horizontal_points)
      it += AddToBuffer<uint32_t>(it, &width);
    }

    DEBUG_ASSERT((data.size() - header_offset) % sizeof(data::Color) == 0);

    // parse depth
    auto bgra_view = image::ImageView::MakeViewFromBuffer<boost::gil::bgra8c_pixel_t>(
        data.begin() + header_offset,
        width,
        height);

    auto float_view = image::ImageView::MakeColorConvertedView<decltype(bgra_view),
        boost::gil::gray32f_pixel_t>(
        bgra_view,
        image::ColorConverter::Depth());

    // convert pixels to 3d
    for (size_t y = 0u; y < height; ++y) {
      for (size_t x = 0u; x < width; ++x) { /// @todo: change to frame_width
        geom::Location point_3d = convert_2d_to_3d(
            fov,
            1000.f, // maximum depth harcoded in shader
            width,
            height,
            x,
            y,
            float_view(x, y)[0]); // normalized depth on pixel (x,y)

        screen_to_sensor_coordinates(point_3d);

        // rotate to the actual horizontal lidar angle (yaw)
        geom::Transform(
            geom::Location(0.0f, 0.0f, 0.0f),
            geom::Rotation(
            0.0f,
            horizontal_angle,
            0.0f)).TransformPoint(point_3d);

        it += AddToBuffer<geom::Location>(it, &point_3d);
      }
    }

    DEBUG_ASSERT(it == buf.end());

    return SharedPtr<data::LidarMeasurement>(
        new data::LidarMeasurement(RawData(std::move(buf))));
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
