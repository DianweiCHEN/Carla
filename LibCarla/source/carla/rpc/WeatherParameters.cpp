// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace rpc {

  using WP = WeatherParameters;

//                          cloudiness   precip.  prec.dep.     wind     azimuth   altitude  fog dens  fog dist  fog fall  wetness  scat.i  mie.scat.s rayleigh.scat.scale
  WP WP::Default         = {   -1.0f,    -1.0f,     -1.0f,    -1.00f,     -1.0f,     -1.0f,    -1.0f,     -1.0f,    -1.0f,   -1.0f,  -1.0f,   -1.0f,     -1.0f};
  WP WP::ClearNoon       = {   10.0f,     0.0f,      0.0f,     10.0f,     -1.0f,     45.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::CloudyNoon      = {   40.0f,     0.0f,      0.0f,     10.0f,     -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetNoon         = {   10.0f,     0.0f,     50.0f,     10.0f,     -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetCloudyNoon   = {   40.0f,     0.0f,     50.0f,     10.0f,     -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::MidRainyNoon    = {   40.0f,    60.0f,     60.0f,     60.0f,     -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::HardRainNoon    = {  100.0f,   100.0f,     90.0f,    100.0f,     -1.0f,     45.0f,     7.0f,     0.75f,     0.1f,    0.0f,   0.0f,   0.03f,   0.0331f};
  WP WP::SoftRainNoon    = {   30.0f,    30.0f,     50.0f,     30.0f,     -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::ClearSunset     = {   10.0f,     0.0f,      0.0f,     10.0f,     -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::CloudySunset    = {   40.0f,     0.0f,      0.0f,     10.0f,     -1.0f,     15.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetSunset       = {   10.0f,     0.0f,     50.0f,     10.0f,     -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetCloudySunset = {   40.0f,     0.0f,     50.0f,     10.0f,     -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::MidRainSunset   = {   40.0f,    60.0f,     60.0f,     60.0f,     -1.0f,     15.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::HardRainSunset  = {  100.0f,   100.0f,     90.0f,    100.0f,     -1.0f,     15.0f,     7.0f,     0.75f,     0.1f,    0.0f,   0.0f,   0.03f,   0.0331f};
  WP WP::SoftRainSunset  = {   30.0f,    30.0f,     50.0f,     30.0f,     -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::ClearNight      = {   10.0f,     0.0f,      0.0f,     10.0f,     -1.0f,    -90.0f,    60.0f,     75.0f,     1.0f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::CloudyNight     = {   40.0f,     0.0f,      0.0f,     10.0f,     -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetNight        = {   10.0f,     0.0f,     50.0f,     10.0f,     -1.0f,    -90.0f,    60.0f,     75.0f,     1.0f,   60.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetCloudyNight  = {   40.0f,     0.0f,     50.0f,     10.0f,     -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,   60.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::SoftRainNight   = {   30.0f,    30.0f,     50.0f,     30.0f,     -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,   60.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::MidRainyNight   = {   80.0f,    60.0f,     60.0f,     60.0f,     -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,   80.0f,   0.0f,   0.03f,   0.0331f};
  WP WP::HardRainNight   = {  100.0f,   100.0f,     90.0f,    100.0f,     -1.0f,    -90.0f,   100.0f,     0.75f,     0.1f,  100.0f,   0.0f,   0.03f,   0.0331f};
} // namespace rpc
} // namespace carla
