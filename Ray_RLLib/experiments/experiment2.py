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

SERVER_VIEW_CONFIG = {
}

SENSOR_CONFIG = {
    "CAMERA_NORMALIZED": [True], # apparently doesnt work if set to false, its just for the image!
    "CAMERA_GRAYSCALE": [True],
    "FRAMESTACK": 4,
}

BIRDVIEW_CONFIG = {
    "SIZE": 190,
    "RADIUS": 15,
    "FRAMESTACK": 4
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
    "n_vehicles": 40,
    "n_walkers": 15,
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
        self.frame_stack = 4  # can be 1,2,3,4
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
            dtype=np.float32,
        )
        self.observation_space = image_space

    def process_observation(self, core, observation):
        """
        Process observations according to your experiment
        :param core:
        :param observation:
        :return:
        """
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


    def compute_reward(self, core, observation):
        """
        Reward function
        :param observation:
        :param core:
        :return:
        """
        c = float(np.sqrt(np.square(self.hero.get_location().x - self.start_location_x) + \
                            np.square(self.hero.get_location().y - self.start_location_y)))

        # if self.observation["collision"] != False:
        #     reward = -10
        # elif self.current_w is not None:
        #     if not(self.current_w.lane_type in self.allowed_types):
        #         reward = -5
        if c > self.previous_distance + 1e-2:
            reward = c - self.previous_distance
        else:
            reward = 0
        self.previous_distance = c
        if c > 30: # to avoid losing points for getting closer to initial location
            self.start_location_x = self.hero.get_location().x
            self.start_location_x = self.hero.get_location().x
            self.previous_distance = 0
        # if self.previous_distance < 15 and reward < 0:
        #     reward = 0
        #     print("avoid negative reward")
        #print(reward)
        return reward

    def spawn_hero(self, world, transform, autopilot=False):

        self.spawn_points = world.get_map().get_spawn_points()
        gc.collect()
        self.hero_blueprints = world.get_blueprint_library().find(self.hero_model)
        self.hero_blueprints.set_attribute("role_name", "hero")

        if self.hero is not None:
            self.hero.destroy()
            self.hero = None
        i = 0
        random.shuffle(self.spawn_points, random.random)
        while True:
            next_spawn_point = self.spawn_points[i % len(self.spawn_points)]
            self.hero = world.try_spawn_actor(self.hero_blueprints, next_spawn_point)
            if self.hero is not None:
                break
            else:
                print("Could not spawn Hero, changing spawn point")
                i+=1

        world.tick()
        print("Hero spawned!")
        self.start_location_x = self.spawn_points[i].location.x
        self.start_location_y = self.spawn_points[i].location.y
        self.past_action = carla.VehicleControl(0.0, 0.00, 0.0, False, False)
