// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/PedNav.h>
#include "carla/geom/Location.h"

void export_pednav() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;

  class_<cc::PedNav, boost::noncopyable, boost::shared_ptr<cc::PedNav>>("PedNav")
    .def(init<>())
    .def("load_navigation_file", CALL_WITHOUT_GIL_1(cc::PedNav, LoadNavigationFile, std::string), (arg("filename")))
    .def("is_location_reachable_by_pedestrian", CALL_WITHOUT_GIL_3(cc::PedNav, IsLocationReachableByPedestrian, cg::Location, cg::Location, float), (arg("from"), arg("to"), arg("max_distance")))
    .def("get_random_location_from_navigation", CALL_RETURNING_OPTIONAL_WITHOUT_GIL(cc::PedNav, GetRandomLocationFromNavigation))
    // .def(self_ns::str(self_ns::self))
  ;
}
