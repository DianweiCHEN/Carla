// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Geometry.h"

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector2D.h"

#include <boost/array.hpp>
#include <boost/math/tools/rational.hpp>

//#include <cephes/fresnel.h>
#include <odrSpiral/odrSpiral.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace carla {
namespace road {
namespace element {

  void DirectedPoint::ApplyLateralOffset(float lateral_offset) {
    /// @todo Z axis??
    auto normal_x =  std::sin(static_cast<float>(tangent));
    auto normal_y = -std::cos(static_cast<float>(tangent));
    location.x += lateral_offset * normal_x;
    location.y += lateral_offset * normal_y;
  }

  DirectedPoint GeometryLine::PosFromDist(double dist) const {
    DEBUG_ASSERT(_length > 0.0);
    dist = geom::Math::Clamp(dist, 0.0, _length);
    DirectedPoint p(_start_position, _heading);
    p.location.x += static_cast<float>(dist * std::cos(p.tangent));
    p.location.y += static_cast<float>(dist * std::sin(p.tangent));
    return p;
  }

  DirectedPoint GeometryArc::PosFromDist(double dist) const {
    dist = geom::Math::Clamp(dist, 0.0, _length);
    DEBUG_ASSERT(_length > 0.0);
    DEBUG_ASSERT(std::fabs(_curvature) > 1e-15);
    const double radius = 1.0 / _curvature;
    constexpr double pi_half = geom::Math::Pi<double>() / 2.0;
    DirectedPoint p(_start_position, _heading);
    p.location.x += static_cast<float>(radius * std::cos(p.tangent + pi_half));
    p.location.y += static_cast<float>(radius * std::sin(p.tangent + pi_half));
    p.tangent += dist * _curvature;
    p.location.x -= static_cast<float>(radius * std::cos(p.tangent + pi_half));
    p.location.y -= static_cast<float>(radius * std::sin(p.tangent + pi_half));
    return p;
  }

  geom::Vector2D RotatebyAngle(double angle, double x, double y){
    const double cos_a = std::cos(angle);
    const double sin_a = std::sin(angle);
    return geom::Vector2D(
        static_cast<float>(x * cos_a - y * sin_a),
        static_cast<float>(y * cos_a + x * sin_a));
  }

  DirectedPoint GeometrySpiral::PosFromDist(double dist) const {
    dist = geom::Math::Clamp(dist, 0.0, _length);
    DEBUG_ASSERT(_length > 0.0);
    DirectedPoint p(_start_position, _heading);

    const double curve_end = (_curve_end);
    const double curve_start = (_curve_start);
    const double curve_dot = (curve_end - curve_start) / (_length);
    const double s_o = curve_start / curve_dot;
    //const double s_f = curve_end / curve_dot;
    double s = s_o + dist;

    double x;
    double y;
    double t;
    odrSpiral(s, curve_dot, &x, &y, &t);

    double x_o;
    double y_o;
    double t_o;
    odrSpiral(s_o, curve_dot, &x_o, &y_o, &t_o);

    x = x - x_o;
    y = y - y_o;
    t = t - t_o;

    geom::Vector2D pos = RotatebyAngle(_heading - t_o, x, y);
    p.location.x += pos.x;
    p.location.y += pos.y;
    p.tangent = _heading + t;

    return p;
  }

  /// @todo
  std::pair<float, float> GeometrySpiral::DistanceTo(const geom::Location &location) const {
    //Not analytic, discretize and find nearest point
    //throw_exception(std::runtime_error("not implemented"));
    return {location.x - _start_position.x, location.y - _start_position.y};
  }

  DirectedPoint GeometryPoly3::PosFromDist(double dist) const{
    double u = dist;
    double v = _a + _b*u + _c*u*u + _d*u*u*u;

    double cos_hdg = std::cos(_heading);
    double sin_hdg = std::cos(_heading);

    double x = u * cos_hdg - v * sin_hdg;
    double y = u * sin_hdg + v * cos_hdg;

    double tangentv = _b + 2 * _c * u + 3 * _d * u * u;
    double theta = std::atan2(tangentv, 1.0);

    DirectedPoint p(_start_position, _heading + theta);
    p.location.x += static_cast<float>(x);
    p.location.y += static_cast<float>(y);
    return p;
  }

  std::pair<float, float> GeometryPoly3::DistanceTo(const geom::Location &/*p*/) const{
    //No analytical expression (Newton-Raphson?/point search)
    //throw_exception(std::runtime_error("not implemented"));
    return {_start_position.x,_start_position.y};
  }

  DirectedPoint GeometryParamPoly3::PosFromDist(const double dist) const
  {
    //from map repo
    double p = dist;
    if(!_arcLength){
      p = geom::Math::Clamp(dist / _length, -1.0, 1.0);
    }

    auto polyU = boost::array<double, 4>{{_aU, _bU, _cU, _dU}};
    auto polyV = boost::array<double, 4>{{_aV, _bV, _cV, _dV}};

    double u = boost::math::tools::evaluate_polynomial(polyU, p);
    double v = boost::math::tools::evaluate_polynomial(polyV, p);

    const double cos_t = std::cos(_heading);
    const double sin_t = std::sin(_heading);

    double x = u * cos_t - v * sin_t;
    double y = u * sin_t + v * cos_t;

    auto tangentPolyU = boost::array<double, 4>{{_bU, 2.0 * _cU, 3.0 * _dU, 0.0}};
    auto tangentPolyV = boost::array<double, 4>{{_bV, 2.0 * _cV, 3.0 * _dV, 0.0}};

    double tangentU = boost::math::tools::evaluate_polynomial(tangentPolyU, p);
    double tangentV = boost::math::tools::evaluate_polynomial(tangentPolyV, p);
    double theta = atan2(tangentV, tangentU);

    DirectedPoint point(_start_position, _heading + theta);
    point.location.x += static_cast<float>(x);
    point.location.y += static_cast<float>(y);
    return point;
  }
  std::pair<float, float> GeometryParamPoly3::DistanceTo(const geom::Location &) const {
    //No analytical expression (Newton-Raphson?/point search)
    //throw_exception(std::runtime_error("not implemented"));
    return {_start_position.x,_start_position.y};
  }
} // namespace element
} // namespace road
} // namespace carla
