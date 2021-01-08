import random
from enum import Enum
from itertools import cycle
import math
import carla
import numpy as np
from gym.spaces import Discrete, Box
import logging
from helper.CarlaHelper import spawn_vehicle_at, post_process_image


class SensorsTransformEnum(Enum):
    Transform_A = 0  # (carla.Transform(carla.Location(x=-5.5, z=2.5), carla.Rotation(pitch=8.0)), Attachment.SpringArm)
    Transform_B = 1  # (carla.Transform(carla.Location(x=1.6, z=1.7)), Attachment.Rigid),
    Transform_c = 2  # (carla.Transform(carla.Location(x=5.5, y=1.5, z=1.5)), Attachment.SpringArm),
    Transform_D = 3  # (carla.Transform(carla.Location(x=-8.0, z=6.0), carla.Rotation(pitch=6.0)), Attachment.SpringArm)
    Transform_E = 4  # (carla.Transform(carla.Location(x=-1, y=-bound_y, z=0.5)), Attachment.Rigid)]


class SensorsEnum(Enum):
    CAMERA_RGB = 0
    CAMERA_DEPTH_RAW = 1
    CAMERA_DEPTH_GRAY = 2
    CAMERA_DEPTH_LOG = 3
    CAMERA_SEMANTIC_RAW = 4
    CAMERA_SEMANTIC_CITYSCAPE = 5
    LIDAR = 6
    CAMERA_DYNAMIC_VISION = 7
    CAMERA_DISTORTED = 8


BASE_SERVER_VIEW_CONFIG = {
    "server_view_x_offset": 00,
    "server_view_y_offset": 00,
    "server_view_height": 200,
    "server_view_pitch": -90,
}

BASE_SENSOR_CONFIG = {
    "SENSOR": [SensorsEnum.CAMERA_DEPTH_RAW],
    "SENSOR_TRANSFORM": [SensorsTransformEnum.Transform_A],
    "CAMERA_X": 84,
    "CAMERA_Y": 84,
    "CAMERA_FOV": 60,
    "CAMERA_NORMALIZED": [True],
    "CAMERA_GRAYSCALE": [True],
    "FRAMESTACK": 1,
}
BASE_OBSERVATION_CONFIG = {
    "CAMERA_OBSERVATION": [False],
    "COLLISION_OBSERVATION": True,
    "LOCATION_OBSERVATION": True,
    "RADAR_OBSERVATION": False,
    "IMU_OBSERVATION": False,
    "LANE_OBSERVATION": True,
    "GNSS_OBSERVATION": False,
}
BASE_EXPERIMENT_CONFIG = {
    "OBSERVATION_CONFIG": BASE_OBSERVATION_CONFIG,
    "Server_View": BASE_SERVER_VIEW_CONFIG,
    "SENSOR_CONFIG": BASE_SENSOR_CONFIG,
    "server_map": "Town02",
    "quality_level": "Low",  # options are low or Epic #ToDO. This does not do anything + change to enum
    "Disable_Rendering_Mode": False,  # If you disable, you will not get camera images
    "n_vehicles": 0,
    "n_walkers": 0,
    "start_pos_spawn_id": 100,  # 82,
    "end_pos_spawn_id": 45,  # 34,
    "hero_vehicle_model": "vehicle.lincoln.mkz2017",
    "Weather": carla.WeatherParameters.ClearNoon,
    "RANDOM_RESPAWN": False,  # Actors are randomly Respawned or Not
    "DISCRETE_ACTION": True,
    "Debug": False,
}

DISCRETE_ACTIONS_SMALL = {
    0: [0.0, 0.00, 1.0, False, False],  # Apply Break
    1: [1.0, 0.00, 0.0, False, False],  # Straight
    2: [1.0, -0.70, 0.0, False, False],  # Right + Accelerate
    3: [1.0, -0.50, 0.0, False, False],  # Right + Accelerate
    4: [1.0, -0.30, 0.0, False, False],  # Right + Accelerate
    5: [1.0, -0.10, 0.0, False, False],  # Right + Accelerate
    6: [1.0, 0.10, 0.0, False, False],  # Left+Accelerate
    7: [1.0, 0.30, 0.0, False, False],  # Left+Accelerate
    8: [1.0, 0.50, 0.0, False, False],  # Left+Accelerate
    9: [1.0, 0.70, 0.0, False, False],  # Left+Accelerate
    10: [0.0, -0.70, 0.0, False, False],  # Left+Stop
    11: [0.0, -0.23, 0.0, False, False],  # Left+Stop
    12: [0.0, 0.23, 0.0, False, False],  # Right+Stop
    13: [0.0, 0.70, 0.0, False, False],  # Right+Stop
}

DISCRETE_ACTIONS_SMALLER = {
    0: [0.0, 0.00, 0.0, False, False], # Coast
    1: [0.0, -0.10, 0.0, False, False], # Turn Left
    2: [0.0, 0.10, 0.0, False, False], # Turn Right
    3: [0.05, 0.00, 0.0, False, False], # Accelerate
    4: [-0.1, 0.00, 0.0, False, False], # Decelerate
    5: [0.0, 0.00, 1.0, False, False], # Brake
    6: [0.05, 0.10, 0.0, False, False], # Turn Right + Accelerate
    7: [0.05, -0.10, 0.0, False, False], # Turn Left + Accelerate
    8: [-0.1, 0.10, 0.0, False, False], # Turn Right + Decelerate
    9: [-0.1, -0.10, 0.0, False, False], # Turn Left + Decelerate
}

DISCRETE_ACTIONS = DISCRETE_ACTIONS_SMALLER

class BaseExperiment:
    def __init__(self, config=BASE_EXPERIMENT_CONFIG):
        self.experiment_config = config
        self.observation = {}
        self.observation["camera"] = []
        self.observation_space = None
        self.action = None
        self.action_space = None

        self.hero = None
        self.spectator = None

        self.spawn_point_list = []
        self.vehicle_list = []
        self.start_location = None
        self.end_location = None

        self.hero_model = ''.join(self.experiment_config["hero_vehicle_model"])

        self.set_observation_space()
        self.set_action_space()


    def get_experiment_config(self):

        return self.experiment_config

    def set_observation_space(self):

        """
        observation_space_option: Camera Image
        :return: observation space:
        """
        raise NotImplementedError

    def get_observation_space(self):

        """
        :return: observation space
        """
        return self.observation_space

    def set_action_space(self):

        """
        :return: None. In this experiment, it is a discrete space
        """
        self.action_space = Discrete(len(DISCRETE_ACTIONS))

    def get_action_space(self):

        """
        :return: action_space. In this experiment, it is a discrete space
        """
        return self.action_space

    def respawn_actors(self, world):

        random_respawn = self.experiment_config["RANDOM_RESPAWN"]

        # Get all spawn points
        spawn_points = list(world.get_map().get_spawn_points())

        randomized_vehicle_spawn_point = spawn_points.copy()
        random.shuffle(randomized_vehicle_spawn_point, random.random)
        randomized_spawn_list = cycle(randomized_vehicle_spawn_point)

        # ToDo remove hero from this list. This should be already done if no random_respawn is False
        for i in range(len(self.spawn_point_list)):
            self.vehicle_list[i].set_autopilot(False)
            self.vehicle_list[i].set_velocity(carla.Vector3D(0, 0, 0))

            if random_respawn is True:
                next_spawn_point = next(randomized_spawn_list)
            else:
                next_spawn_point = self.spawn_point_list[i]
            self.vehicle_list[i].set_transform(next_spawn_point)

            # Reset the autopilot
            self.vehicle_list[i].set_autopilot(False)
            self.vehicle_list[i].set_autopilot(True)

    def spawn_actors(self, core, hybrid=False, seed=None, max_time=0.1):

        """
        This experiment spawns vehicles randomly on a map based on a pre-set number of vehicles.
        To spawn, the spawn points and randomized and the vehicles are spawned with a each vehicle occupying a
        single spawn point to avoid vehicles running on top of each other
        This experiment does not spawn any vehicle where the actor is to be spawned
        :param core:
        :return:
        """

        world = core.get_core_world()
        client = core.get_core_client()
        traffic_manager = client.get_trafficmanager()
        if hybrid:
            traffic_manager.set_hybrid_physics_mode(True)
        if seed is not None:
            traffic_manager.set_random_device_seed(seed)
        traffic_manager.set_synchronous_mode(True)

        # Get a list of all the vehicle blueprints
        vehicle_blueprints = world.get_blueprint_library().filter("vehicle.*")
        car_blueprints = [
            x
            for x in vehicle_blueprints
            if int(x.get_attribute("number_of_wheels")) == 4
        ]
        walker_blueprints = world.get_blueprint_library().filter("walker.*")

        # Now we are ready to spawn all the vehicles (except the hero)
        n_vehicles = self.experiment_config["n_vehicles"]
        n_walkers = self.experiment_config["n_walkers"]

        spawn_points = world.get_map().get_spawn_points()
        number_of_spawn_points = len(spawn_points)

        if n_vehicles < number_of_spawn_points:
            random.shuffle(spawn_points)
        elif n_vehicles > number_of_spawn_points:
            msg = 'requested %d vehicles, but could only find %d spawn points'
            logging.warning(msg, n_vehicles, number_of_spawn_points)
            n_vehicles = number_of_spawn_points

        # @todo cannot import these directly.
        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor

        walkers_list = []
        batch = []
        vehicles_list = []
        all_id = []
        for n, transform in enumerate(spawn_points):
            if n >= n_vehicles:
                break
            blueprint = random.choice(car_blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            blueprint.set_attribute('role_name', 'autopilot')

            # spawn the cars and set their autopilot and light state all together
            batch.append(SpawnActor(blueprint, transform)
                .then(SetAutopilot(FutureActor, True, traffic_manager.get_port())))

        for response in client.apply_batch_sync(batch, True):
            if response.error:
                logging.error(response.error)
            else:
                vehicles_list.append(response.actor_id)
        percentagePedestriansRunning = 0.0      # how many pedestrians will run
        percentagePedestriansCrossing = 0.0     # how many pedestrians will walk through the road
        # 1. take all the random locations to spawn
        w_spawn_points = []
        for i in range(n_walkers):
            spawn_point = carla.Transform()
            loc = world.get_random_location_from_navigation()
            if (loc != None):
                spawn_point.location = loc
                w_spawn_points.append(spawn_point)
        # 2. we spawn the walker object
        batch = []
        walker_speed = []
        for spawn_point in w_spawn_points:
            walker_bp = random.choice(walker_blueprints)
            # set as not invincible
            if walker_bp.has_attribute('is_invincible'):
                walker_bp.set_attribute('is_invincible', 'false')
            # set the max speed
            if walker_bp.has_attribute('speed'):
                if (random.random() > percentagePedestriansRunning):
                    # walking
                    walker_speed.append(walker_bp.get_attribute('speed').recommended_values[1])
                else:
                    # running
                    walker_speed.append(walker_bp.get_attribute('speed').recommended_values[2])
            else:
                print("Walker has no speed")
                walker_speed.append(0.0)
            batch.append(SpawnActor(walker_bp, spawn_point))
        results = client.apply_batch_sync(batch, True)
        walker_speed2 = []
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list.append({"id": results[i].actor_id})
                walker_speed2.append(walker_speed[i])
        walker_speed = walker_speed2
        # 3. we spawn the walker controller
        batch = []
        walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
        for i in range(len(walkers_list)):
            batch.append(SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
        results = client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list[i]["con"] = results[i].actor_id
        # 4. we put altogether the walkers and controllers id to get the objects from their id
        for i in range(len(walkers_list)):
            all_id.append(walkers_list[i]["con"])
            all_id.append(walkers_list[i]["id"])
        all_actors = world.get_actors(all_id)

        world.tick()

        # 5. initialize each controller and set target to walk to (list is [controler, actor, controller, actor ...])
        # set how many pedestrians can cross the road
        world.set_pedestrians_cross_factor(percentagePedestriansCrossing)
        for i in range(0, len(all_id), 2):
            # start walker
            all_actors[i].start()
            # set walk to random point
            all_actors[i].go_to_location(world.get_random_location_from_navigation())
            # max speed
            all_actors[i].set_max_speed(float(walker_speed[int(i/2)]))

        world.tick()

        self.start_location = spawn_points[self.experiment_config["start_pos_spawn_id"]]
        self.end_location = spawn_points[self.experiment_config["end_pos_spawn_id"]]


    def set_server_view(self,core):

        """
        Set server view to be behind the hero
        :param core:Carla Core
        :return:
        """
        # spectator following the car
        transforms = self.hero.get_transform()
        server_view_x = self.hero.get_location().x - 5 * transforms.get_forward_vector().x
        server_view_y = self.hero.get_location().y - 5 * transforms.get_forward_vector().y
        server_view_z = self.hero.get_location().z + 3
        server_view_pitch = transforms.rotation.pitch
        server_view_yaw = transforms.rotation.yaw
        server_view_roll = transforms.rotation.roll
        self.spectator = core.get_core_world().get_spectator()
        self.spectator.set_transform(
            carla.Transform(
                carla.Location(x=server_view_x, y=server_view_y, z=server_view_z),
                carla.Rotation(pitch=server_view_pitch,yaw=server_view_yaw,roll=server_view_roll),
            )
        )

    def get_speed(self):
        """
        Compute speed of a vehicle in Km/h.

            :param vehicle: the vehicle for which speed is calculated
            :return: speed as a float in Km/h
        """
        vel = self.hero.get_velocity()

        return 3.6 * math.sqrt(vel.x ** 2 + vel.y ** 2 + vel.z ** 2)

    def get_done_status(self):
        done = (self.observation["collision"] is not False) or self.observation["lane"] or (self.get_speed() > self.hero.get_speed_limit()+10.0)
        return done

    def process_observation(self, core, observation):

        """
        Main function to do all the post processing of observations. This is an example.
        :param core:
        :param observation:
        :return:
        """
        observation['camera'] = post_process_image(
                                            observation['camera'],
                                            normalized = self.experiment_config["SENSOR_CONFIG"]["CAMERA_NORMALIZED"][0],
                                            grayscale = self.experiment_config["SENSOR_CONFIG"]["CAMERA_GRAYSCALE"][0]
            )

        return observation

    def get_observation(self, core):

        info = {}
        for i in range(0,len(self.experiment_config["SENSOR_CONFIG"]["SENSOR"])):
            if len(self.experiment_config["SENSOR_CONFIG"]["SENSOR"]) != len(self.experiment_config["OBSERVATION_CONFIG"]["CAMERA_OBSERVATION"]):
                raise Exception("You need to specify the CAMERA_OBSERVATION for each sensor.")
            if self.experiment_config["OBSERVATION_CONFIG"]["CAMERA_OBSERVATION"][i]:
                self.observation['camera'].append(core.get_camera_data())
        if self.experiment_config["OBSERVATION_CONFIG"]["COLLISION_OBSERVATION"]:
            self.observation["collision"] = core.get_collision_data()
        if self.experiment_config["OBSERVATION_CONFIG"]["LOCATION_OBSERVATION"]:
            self.observation["location"] = self.hero.get_transform()
        if self.experiment_config["OBSERVATION_CONFIG"]["LANE_OBSERVATION"]:
            self.observation["lane"] = core.get_lane_data()
        if self.experiment_config["OBSERVATION_CONFIG"]["GNSS_OBSERVATION"]:
            self.observation["gnss"] = core.get_gnss_data()
        if self.experiment_config["OBSERVATION_CONFIG"]["IMU_OBSERVATION"]:
            self.observation["imu"] = core.get_imu_data()
        if self.experiment_config["OBSERVATION_CONFIG"]["RADAR_OBSERVATION"]:
            self.observation["radar"] = core.get_radar_data()

        info["control"] = {
            "steer": self.action.steer,
            "throttle": self.action.throttle,
            "brake": self.action.brake,
            "reverse": self.action.reverse,
            "hand_brake": self.action.hand_brake,
        }

        return self.observation, info

    def update_measurements(self, core):

        for i in range(0,len(self.experiment_config["SENSOR_CONFIG"]["SENSOR"])):
            if len(self.experiment_config["SENSOR_CONFIG"]["SENSOR"]) != len(self.experiment_config["OBSERVATION_CONFIG"]["CAMERA_OBSERVATION"]):
                raise Exception("You need to specify the CAMERA_OBSERVATION for each sensor.")
            if self.experiment_config["OBSERVATION_CONFIG"]["CAMERA_OBSERVATION"][i]:
                core.update_camera()
        if self.experiment_config["OBSERVATION_CONFIG"]["COLLISION_OBSERVATION"]:
            core.update_collision()
        if self.experiment_config["OBSERVATION_CONFIG"]["LANE_OBSERVATION"]:
            core.update_lane_invasion()


    def update_actions(self, action, hero):
        if action is None:
            self.action = carla.VehicleControl()
        else:
            action = DISCRETE_ACTIONS[int(action)]
            self.action.brake = float(np.clip(action[2], 0, 1))
            if action[2] != 0.0:
                self.action.throttle = float(0)
            else:
                self.action.throttle = float(np.clip(self.past_action.throttle + action[0], 0, 1))
            self.action.steer = float(np.clip(self.past_action.steer + action[1], -0.7, 0.7))
            self.action.reverse = action[3]
            self.action.hand_brake = action[4]
            self.past_action = self.action
            self.hero.apply_control(self.action)

    def compute_reward(self, core, observation):

        """
        :param core:
        :param observation:
        :return:
        """

        print("This is a base experiment. Make sure you make you own reward computing function")
        return NotImplementedError

    def initialize_reward(self, core):

        """
        Generic initialization of reward function
        :param core:
        :return:
        """
        print("This is a base experiment. Make sure you make you own reward initialization function")
        raise NotImplementedError


    # ==============================================================================
    # -- Hero -----------------------------------------------------------
    # ==============================================================================
    def spawn_hero(self, core, transform, autopilot=False):

        """
        This function spawns the hero vehicle. It makes sure that if a hero exists, it destroys the hero and respawn
        :param core:
        :param transform: Hero location
        :param autopilot: Autopilot Status
        :return:
        """
        world = core.get_core_world()

        if self.hero is not None:
            self.hero.destroy()
            self.hero = None

        hero_car_blueprint = world.get_blueprint_library().find(self.hero_model)
        hero_car_blueprint.set_attribute("role_name", "hero")

        while self.hero is None:
            self.hero = world.try_spawn_actor(hero_car_blueprint, transform)

        self.past_action = carla.VehicleControl(0.0, 0.00, 0.0, False, False)

    def get_hero(self):

        """
        Get hero vehicle
        :return:
        """
        return self.hero

    # ==============================================================================
    # -- Tick -----------------------------------------------------------
    # ==============================================================================

    def experiment_tick(self, core, action):

        """
        This is the "tick" logic.
        :param core:
        :param action:
        :return:
        """

        world = core.get_core_world()
        world.tick()
        self.update_measurements(core)
        self.update_actions(action, self.hero)

