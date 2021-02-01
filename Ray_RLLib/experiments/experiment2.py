from experiments.base_experiment import *
from helper.CarlaHelper import spawn_vehicle_at, post_process_image, update_config
import random
import numpy as np
from gym.spaces import Box
from itertools import cycle
import cv2
import time
import carla
import gc
from PIL import Image
import sys
import math

SERVER_VIEW_CONFIG = {
}

SENSOR_CONFIG = {
    "CAMERA_NORMALIZED": [True], # apparently doesnt work if set to false, its just for the image!
    "CAMERA_GRAYSCALE": [True],
    "FRAMESTACK": 4,
}

BIRDVIEW_CONFIG = {
    "SIZE": 300,
    "RADIUS": 15,
    "FRAMESTACK": 1
}

OBSERVATION_CONFIG = {
    "CAMERA_OBSERVATION": [False],
    "BIRDVIEW_OBSERVATION": True,
}

EXPERIMENT_CONFIG = {
    "OBSERVATION_CONFIG": OBSERVATION_CONFIG,
    "Server_View": SERVER_VIEW_CONFIG,
    "SENSOR_CONFIG": SENSOR_CONFIG,
    "server_map": "Town02_Opt",
    "BIRDVIEW_CONFIG": BIRDVIEW_CONFIG,
    "n_vehicles": 0,
    "n_walkers": 0,
    "hero_vehicle_model": "vehicle.lincoln.mkz2017",
}

class Experiment(BaseExperiment):
    def __init__(self):
        config=update_config(BASE_EXPERIMENT_CONFIG, EXPERIMENT_CONFIG)
        super().__init__(config)

    def initialize_reward(self, core):
        """
        Generic initialization of reward function
        :param core:
        :return:
        """
        self.previous_distance = 0
        self.i = 0
        self.frame_stack = 1  # can be 1,2,3,4
        self.prev_image_0 = None
        self.prev_image_1 = None
        self.prev_image_2 = None
        self.allowed_types = [carla.LaneType.Driving, carla.LaneType.Parking]

    def set_observation_space(self):
        num_of_channels = 3
        image_space = Box(
            low=0.0,
            high=255.0,
            shape=(
                self.experiment_config["BIRDVIEW_CONFIG"]["SIZE"],
                self.experiment_config["BIRDVIEW_CONFIG"]["SIZE"],
                num_of_channels * self.experiment_config["BIRDVIEW_CONFIG"]["FRAMESTACK"],
            ),
            dtype=np.uint8,
        )
        self.observation_space = image_space

    def process_observation(self, core, observation):
        """
        Process observations according to your experiment
        :param core:
        :param observation:
        :return:
        """
        # if self.i % 1000 == 0:
        #     img = Image.fromarray(observation["birdview"], 'RGB')
        #     img.show()
        # self.i += 1

        self.set_server_view(core)
        image = post_process_image(observation['birdview'],
                                   normalized = False,
                                   grayscale = False
        )

        if self.prev_image_0 is None:
            self.prev_image_0 = image
            self.prev_image_1 = self.prev_image_0
            self.prev_image_2 = self.prev_image_1

        images = image

        if self.frame_stack >= 2:
            images = np.concatenate([self.prev_image_0, images], axis=2)
        if self.frame_stack >= 3 and images is not None:
            images = np.concatenate([self.prev_image_1, images], axis=2)
        if self.frame_stack >= 4 and images is not None:
            images = np.concatenate([self.prev_image_2, images], axis=2)

        self.prev_image_2 = self.prev_image_1
        self.prev_image_1 = self.prev_image_0
        self.prev_image_0 = image

        return images

    def get_done_status(self):
        #done = self.observation["collision"] is not False or not self.check_lane_type(map)
        # self.done_idle = self.max_idle < self.t_idle
        # if self.get_speed() > 2.0:
        #     self.t_idle = 0
        # self.done_max_time = self.max_ep_time < self.t_ep
        # self.done_falling = self.hero.get_location().z < -0.5
        # if len(self.route) == 0: self.done_route = True
        return self.done_idle or self.done_max_time or self.done_falling or self.done_route

    def inside_lane(self, map):
        self.current_w = map.get_waypoint(self.hero.get_location(), lane_type=carla.LaneType.Any)
        return self.current_w.lane_type in self.allowed_types

    def dist_to_driving_lane(self, map_):
        cur_loc = self.hero.get_location()
        cur_w = map_.get_waypoint(cur_loc)
        return math.sqrt((cur_loc.x - cur_w.transform.location.x)**2 +
                         (cur_loc.y - cur_w.transform.location.y)**2)

    def compute_reward(self, core, observation, map, world):
        """
        Reward function
        :param observation:
        :param core:
        :return:
        """
        def unit_vector(vector):
            """ Returns the unit vector of the vector.  """
            return vector / np.linalg.norm(vector)

        def compute_angle(u, v):
            return -math.atan2(u[0]*v[1] - u[1]*v[0], u[0]*v[0] + u[1]*v[1]);

        def nearest_wp(cur_loc):
            nearest_wp = (0, None, sys.float_info.max)
            for i, wp in enumerate(self.route):
                d = math.sqrt(
                    (wp.transform.location.x - cur_loc.x)**2 +
                    (wp.transform.location.y - cur_loc.y)**2
                )

                wp_to_hero = [
                    cur_loc.x - wp.transform.location.x,
                    cur_loc.y - wp.transform.location.y,
                ]
                wp_heading = wp.transform.get_forward_vector()
                wp_heading = unit_vector([wp_heading.x, wp_heading.y])
                if np.dot(wp_heading, wp_to_hero) < 0:
                    if d < nearest_wp[2]:
                        nearest_wp = (i, wp, d)

            return nearest_wp

#        print("\n\n reward debugging")
#        print(" ===== Accion aplicada: {} =====".format(self.action))
        self.done_idle = self.max_idle < self.t_idle
        if self.get_speed() > 2.0:
            self.t_idle = 0
        self.done_max_time = self.max_ep_time < self.t_ep
        self.done_falling = self.hero.get_location().z < -0.5
        if self.done_max_time:
            print("Done by max time")
            return 10
        if self.done_falling:
            print("Done falling")
            return -100
        if self.done_idle:
            print("Done idle")
            return -50

        reward = 0

        # Current state
        cur_loc = self.hero.get_transform().location
        cur_heading = self.hero.get_transform().get_forward_vector()
        cur_heading = [cur_heading.x, cur_heading.y]
#        print("Current velocity: {}".format(self.get_speed()))
#        print("Current heading: {}".format(cur_heading))

        # ----------------
        # Nearest waypoint
        # ----------------
        nearest_index, nearest_wp, nearest_dist = nearest_wp(cur_loc)

        if nearest_index is None or nearest_dist > 25:
#            if nearest_index is None:
#                print("No nearest waypoint!!!")
#            else:
#                print("Nearest waypoint too far!!!")
            return 0

        # Remove predecessors of the nearest waypoint if necessary.
        if nearest_index > 0:
            self.route = self.route[nearest_index:]

        if nearest_dist < 1:
            # Remove nearest waypoint.
            self.route.pop(0)
            if len(self.route) == 0:  # Reach goal
                self.done_route = True
                return 10
            reward += 5

        # ----------------
        # Target direction
        # ----------------
        target = nearest_wp
        if self.get_speed() > 0.5:
            if nearest_dist < 5 and len(self.route) > 1:
                target = self.route[1]  # +1 nearest waypoint

            hero_to_wp = unit_vector([
                target.transform.location.x - cur_loc.x,
                target.transform.location.y - cur_loc.y
            ])
            angle = compute_angle(cur_heading, hero_to_wp)
#            print("Hero to target waypoint: {}".format(hero_to_wp))
#            print("Angulo target: {}".format(angle))
#            if math.sin(angle) > 0:
#                print("Deberiamos girar un poco a la izquierda")
#            else:
#                print("Deberiamos girar un poco a la derecha")

            if abs(math.sin(angle)) < 0.15:
#                print("Estamos alineados ---> +1")
                reward += 1
            else:
#                print("NO estamos alineados")
                if self.action.steer * math.sin(angle) < 0:
#                    print("Corrigiendo direction: {} -----> +1".format(self.action.steer))
                    reward += 0.5

#        else:
#            print("Not enough velocity...")

        # -------------------------
        # Check inside driving lane
        # -------------------------
        if not self.inside_lane(map):
            # Proportional decrease
            d = self.dist_to_driving_lane(map)
            Dmin, Dmax = 0, 10
            m = max((d - Dmax) / (Dmin - Dmax), 0)
        else:
            # Maximum reward if inside lane
            m = 1

        reward = m*reward

        # -----
        # Debug
        # -----
        # Route
        # for wp in self.route:
        #     if wp == nearest_wp:
        #         size = 0.5
        #         z = 1
        #         color = carla.Color(0, 255, 0)
        #     elif wp == target:
        #         size = 0.5
        #         z = 2
        #         color = carla.Color(255, 0, 0)
        #     else:
        #         z = 0.5
        #         size = 0.2
        #         color = carla.Color(0, 0, 255)

        #     world.debug.draw_point(
        #         wp.transform.location + carla.Location(z), size=size, color=color, life_time=-1.0
        #     )

            # world.debug.draw_line(
            #     cur_loc, nearest_wp.transform.location, thickness=0.1, life_time=0.2)
            # world.debug.draw_line(
            #     cur_loc, target.transform.location, thickness=0.1, life_time=-1.0)

#        print("Reward: {}".format(reward))
        return reward
