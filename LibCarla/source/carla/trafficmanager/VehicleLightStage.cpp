
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/VehicleLightStage.h"

namespace carla {
namespace traffic_manager {

VehicleLightStage::VehicleLightStage(
  const std::vector<ActorId> &vehicle_id_list,
  const SimulationState &simulation_state,
  const BufferMap &buffer_map,
  const Parameters &parameters,
  const cc::World &world,
  ControlFrame& control_frame)
  : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    buffer_map(buffer_map),
    parameters(parameters),
    world(world),
    control_frame(control_frame) {}

void VehicleLightStage::ClearCycleCache() {
  // Get the global weather and all the vehicle light states at once
  all_light_states = world.GetVehiclesLightStates();
  weather = world.GetWeather();
}

void VehicleLightStage::Update(const unsigned long index) {
  ActorId id = vehicle_id_list.at(index);
  rpc::VehicleLightState::flag_type light_states = uint32_t(-1);
  bool brake_lights = false;
  bool left_turn_indicator = false;
  bool right_turn_indicator = false;
  bool position = false;
  bool low_beam = false;
  bool high_beam = false;
  bool fog_lights = false;

  if (!parameters.GetUpdateVehicleLightState(id))
    return; // this vehicle is not set to have automatic lights update

  // search the current light state of the vehicle
  for (auto&& vls : all_light_states) {
    if (vls.first == id) {
      light_states = vls.second;
      break;
    }
  }

  cg::Vector3D actor_vec = simulation_state.GetHeading(id);

  // Recover the planned waypoints for this vehicle
  if (buffer_map.count(id) == 1) {
    const Buffer& waypoint_deque = buffer_map.at(id);
    // Find the next intersection (if any) to decide to turn on the blinkers
    for (const SimpleWaypointPtr& swpp : waypoint_deque) {
      WaypointPtr wptr = swpp->GetWaypoint();
      if (!wptr->IsJunction())
        continue;

      // Get the end of the junction road segment
      std::vector<WaypointPtr> next_wptrs = wptr -> GetNextUntilLaneEnd(2);
      if(next_wptrs.empty())
        break;
      wptr = next_wptrs.back();
      cg::Vector3D next_road_vec = wptr->GetTransform().GetForwardVector();
      cg::Vector3D up_vec(0, 0, 1);
      float dot_prod = actor_vec.x*next_road_vec.x +
                        actor_vec.y*next_road_vec.y +
                        actor_vec.z*next_road_vec.z;
      cg::Vector3D cross_prod(actor_vec.y*up_vec.z - actor_vec.z*up_vec.y,
                              actor_vec.z*up_vec.x - actor_vec.x*up_vec.z,
                              actor_vec.x*up_vec.y - actor_vec.y*up_vec.x);

      float dot_prod_left = cross_prod.x*next_road_vec.x +
                            cross_prod.y*next_road_vec.y +
                            cross_prod.z*next_road_vec.z;

      // Determine if the vehicle is truning left or right
      if(dot_prod < 0.5) {
        if(dot_prod_left > 0.5)
          left_turn_indicator = true;
        if(dot_prod_left < -0.5)
          right_turn_indicator = true;
      }
      break;
    }
  }

  // Determine brake light state
  for (size_t cc = 0; cc < control_frame.size(); cc++) {
    if (control_frame[cc].command.type() == typeid(carla::rpc::Command::ApplyVehicleControl)) {
      carla::rpc::Command::ApplyVehicleControl& ctrl = boost::get<carla::rpc::Command::ApplyVehicleControl>(control_frame[cc].command);
      if (ctrl.actor == id) {
        brake_lights = (ctrl.control.brake > 0.5); // hard braking, avoid blinking for throttle control
        break;
      }
    }
  }

  // Determine position, fog and beams

  // Turn on beams & positions from sunset to dawn
  if (weather.sun_altitude_angle < constants::VehicleLight::SUN_ALTITUDE_DEGREES_BEFORE_DAWN ||
      weather.sun_altitude_angle > constants::VehicleLight::SUN_ALTITUDE_DEGREES_AFTER_SUNSET)
  {
    position = true;
    low_beam = true;
  }
  else if (weather.sun_altitude_angle < constants::VehicleLight::SUN_ALTITUDE_DEGREES_JUST_AFTER_DAWN ||
           weather.sun_altitude_angle > constants::VehicleLight::SUN_ALTITUDE_DEGREES_JUST_BEFORE_SUNSET)
  {
    position = true;
  }
  // Turn on lights under heavy rain
  if (weather.precipitation > constants::VehicleLight::HEAVY_PRECIPITATION_THRESHOLD) {
    position = true;
    low_beam = true;
  }
  // Turn on fog lights
  if (weather.fog_density > constants::VehicleLight::FOG_DENSITY_THRESHOLD) {
    position = true;
    low_beam = true;
    fog_lights = true;
  }

  // Determine the new vehicle light state
  rpc::VehicleLightState::flag_type new_light_states = light_states;
  if (brake_lights)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake);

  if (left_turn_indicator)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker);

  if (right_turn_indicator)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker);

  if (position)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position);

  if (low_beam)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam);

  if (high_beam)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam);

  if (fog_lights)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog);

  // Update the vehicle light state if it has changed
  if (new_light_states != light_states)
    control_frame.push_back(carla::rpc::Command::SetVehicleLightState(id, new_light_states));
}

void VehicleLightStage::RemoveActor(const ActorId) {
}

void VehicleLightStage::Reset() {
}

} // namespace traffic_manager
} // namespace carla
