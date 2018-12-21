// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/GPULidarSerializer.h"

#include "carla/geom/Math.h"
#include "carla/geom/Location.h"
#include "carla/image/ImageView.h"
#include "carla/sensor/data/LidarMeasurement.h"

#include <cmath>

namespace carla {
namespace sensor {
namespace s11n {

  static geom::Location convert_2d_to_3d(
      const float fov,
      const float far,
      const float width,
      const float height,
      const float x,
      const float y,
      const float norm_depth) {

    // K matrix:
    // { {f,   0.0, width  / 2},
    //   {0.0, f,   height / 2},
    //   {0.0, 0.0, 1.0       } }

    // invert K and dot product it by
    // { {x},
    //   {y},
    //   {1} }

    // compute f
    const float f = width / (2.0f * std::tan(
        fov * carla::geom::Math::pi() / 360.0f));

    const geom::Location point_3d(
      (x - (width  / 2.f)) / f,
      (y - (height / 2.f)) / f,
      1.0f);

    return point_3d * far * norm_depth;
  }

  /// Helper function to fill a buffer
  template <typename T>
  size_t AddToBuffer(void *it, const T *data) {
    std::memcpy(it, data, sizeof(T));
    return sizeof(T);
  }

  SharedPtr<SensorData> GPULidarSerializer::Deserialize(RawData data) {
    const auto &sensor_header = data.GetHeader();
    using SensorHeaderType = std::remove_reference_t<decltype(sensor_header)>;
    const auto &lidar_header = DeserializeHeader(data);

    /// convert to 3d points

    /// get the depth image size
    const uint32_t width = lidar_header.max_horizontal_points;
    const uint32_t height = lidar_header.channels;

    const float fov = lidar_header.fov;
    // total points to convert this frame
    // const uint32_t frame_width = lidar_header.current_horizontal_points;

    /// create a buffer with the same format as LidarMeasurement does.
    /// define the buffer size
    Buffer buf(
        sizeof(SensorHeaderType) +
        sizeof(float) + // horizontal angle
        sizeof(uint32_t) + // channel count
        sizeof(uint32_t) * lidar_header.channels + // number of points per channel
        sizeof(geom::Location) * width * lidar_header.channels); // points

    /// copy data...
    auto it = buf.begin();
    it += AddToBuffer<SensorHeaderType>(it, &sensor_header); // header
    it += AddToBuffer<float>(it, &lidar_header.horizontal_angle); // horizontal angle
    it += AddToBuffer<uint32_t>(it, &lidar_header.channels); // num of channels

    // for channel in channels
    for (uint32_t i = 0; i < lidar_header.channels; ++i) {
      // num of points per channel #n (in our case, is always
      // current_horizontal_points)
      it += AddToBuffer<uint32_t>(it, &width);
    }

    DEBUG_ASSERT((data.size() - header_offset) % sizeof(data::Color) == 0);

    // parse depth
    auto bgra_view = image::ImageView::MakeViewFromBuffer<boost::gil::bgra8c_pixel_t>(
        data.begin() + header_offset,
        width,
        height);

    auto float_view = image::ImageView::MakeColorConvertedView<decltype(bgra_view), boost::gil::gray32f_pixel_t>(
        bgra_view,
        image::ColorConverter::Depth());

    // convert to 3d
    for(size_t y = 0u; y < height; ++y) {
      for(size_t x = 0u; x < width; ++x) { // change to frame_width
        using namespace boost::gil;
        const float depth =
             get_color(bgra_view(x, y), red_t()) +
            (get_color(bgra_view(x, y), green_t()) * 256) +
            (get_color(bgra_view(x, y), blue_t())  * 256 * 256);
        const float normalized = depth / static_cast<float>(256 * 256 * 256 - 1);

        geom::Location point_3d = convert_2d_to_3d(
            fov,
            100000.f, // maximum depth harcoded in shader
            width,
            height,
            x,
            y,
            normalized); // normalized depth on pixel (x,y)

        it += AddToBuffer<geom::Location>(it, &point_3d); // location #n
      }
    }

    DEBUG_ASSERT(it == buf.end());

    return SharedPtr<data::LidarMeasurement>(
        new data::LidarMeasurement(RawData(std::move(buf))));

    // return im;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
