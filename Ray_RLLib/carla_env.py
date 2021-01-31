"""
This is a sample carla environment. It does basic functionality.
"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import gym
# from gym.utils import seeding

from core.CarlaCore import CarlaCore


class CarlaEnv(gym.Env):

    def __init__(self, config):

        self.environment_config = config

        module = __import__("experiments.{}".format(self.environment_config["Experiment"] ))
        exec("self.experiment = module.{}.Experiment()".format(self.environment_config["Experiment"]))
        self.action_space = self.experiment.get_action_space()
        self.observation_space = self.experiment.get_observation_space()
        self.experiment_config = self.experiment.get_experiment_config()

        self.core = CarlaCore(self.environment_config, self.experiment_config)
        self.world = self.core.get_core_world()
        CarlaCore.spawn_npcs(self.core, self.experiment_config["n_vehicles"],self.experiment_config["n_walkers"], hybrid = True)
        self.map = self.world.get_map()
        self.reset()

    def reset(self):
        self.core.reset_sensors(self.experiment_config)
        self.experiment.spawn_hero(self.world, self.experiment.start_location, autopilot=False)
        self.experiment.compute_route(self.experiment.get_hero(), self.map)
        self.core.setup_sensors(
            self.experiment.experiment_config,
            self.experiment.get_hero(),
            self.experiment.get_route(),
            self.world.get_settings().synchronous_mode,
        )

        self.experiment.initialize_reward(self.core)
        self.experiment.set_server_view(self.core)
        self.experiment.experiment_tick(self.core, self.world, action=None)
        obs, info = self.experiment.get_observation(self.core)
        obs = self.experiment.process_observation(self.core, obs)
        return obs

    def step(self, action):
        # assert action in [0, 13], action
        self.experiment.experiment_tick(self.core, self.world, action)
        observation, info = self.experiment.get_observation(self.core)
        observation = self.experiment.process_observation(self.core, observation)
        reward = self.experiment.compute_reward(self.core,observation, self.map, self.world)
        done = self.experiment.get_done_status()
        return observation, reward, done, info

#    def seed(self, seed=None):
#        self.np_random, seed = seeding.np_random(seed)
#        return [seed]
