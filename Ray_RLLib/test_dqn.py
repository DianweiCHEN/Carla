"""DQN Algorithm. Tested with CARLA.
You can visualize experiment results in ~/ray_results using TensorBoard.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import ray
from ray import tune
from carla_env import CarlaEnv
from ray.rllib.models.modelv2 import ModelV2
from ray.rllib.models.catalog import ModelCatalog
from ray.rllib.models.tf.fcnet import FullyConnectedNetwork
from ray.tune import grid_search, run_experiments
from helper.CarlaHelper import kill_server
import tensorflow as tf

import gc


ENV_CONFIG = {"RAY": True, "DEBUG_MODE": False}  # Are we running an experiment in Ray

env_config = ENV_CONFIG.copy()
env_config.update(
    {
        "RAY": True,  # Are we running an experiment in Ray
        "DEBUG_MODE": False,
        "Experiment": "experiment2",

    }
)

if __name__ == "__main__":
    print("THIS EXPERIMENT HAS NOT BEEN FULLY TESTED")
    kill_server()
    gc.enable()
    tf.keras.backend.clear_session()
    while True:
        try:
            ray.init()
            run_experiments({
                "dqn": {
                    "run": "APEX",
                    "env": CarlaEnv,
                    #"stop":{"episodes_total":ep}, # {"training_iteration":300},
                    "checkpoint_at_end":True,
                    "checkpoint_freq":20,
                    #"restore": "/home/jacopobartiromo/ray_results/dqn/DQN_CarlaEnv_e261f_00000_0_2020-12-16_23-06-01/checkpoint_860/checkpoint-860",
                    "config": {
                        #"framework" : "torch",
                        "env_config": env_config,
                        "num_gpus_per_worker": 0,
                        "num_cpus_per_worker":3,
                        "buffer_size": 100,
                        "num_workers": 2,
                    },
                },
            },
            resume = False,
            )
        finally:
            ray.shutdown()
