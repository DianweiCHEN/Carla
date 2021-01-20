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
import time

ENV_CONFIG = {"RAY": True, "DEBUG_MODE": False}  # Are we running an experiment in Ray

env_config = ENV_CONFIG.copy()
env_config.update(
    {
        "RAY": True,  # Are we running an experiment in Ray
        "DEBUG_MODE": False,
        "Experiment": "experiment2",

    }
)

def first_run(directory, name, model):
    try:
        kill_server()
        tf.keras.backend.clear_session()
        ray.init()
        run_experiments({
            name: {
                "run": "DQN",
                "env": CarlaEnv,
                "stop": {"perf/ram_util_percent":85.0}, # {"training_iteration":300},
                "checkpoint_at_end":True,
                "local_dir": directory,
                "config": {
                    "env_config": env_config,
                    "num_gpus_per_worker": 0.5,
                    "num_cpus_per_worker":4,
                    "buffer_size": 1000,
                    "num_workers": 1,
                    "model": {
                        'dim': 300,
                        'conv_filters': [
                            [16, [8, 8], 4],
                            [32, [4, 4], 2],
                            [32, [4, 4], 2],
                            [64, [19, 19], 1],
                        ],
                    },
                },
            },
        },
        resume = False,
        )
        ray.shutdown()
        gc.collect()
    finally:
        kill_server()
        ray.shutdown()

def restore_run(directory, name, model):
    to_ignore = []
    try:
        while (True):
            kill_server()
            tf.keras.backend.clear_session()
            # Finds checkpoint dir path
            start = directory+"/"+name
            for f in os.listdir(start):
                if model in f and not(f in to_ignore):
                    start+=("/"+f)
                    to_ignore.append(start)
                    break
            for f in os.listdir(start):
                if "checkpoint" in f:
                    start+=("/"+f+"/"+f.replace("_","-"))
                    break
            ray.init()
            run_experiments({
                name: {
                    "run": model,
                    "env": CarlaEnv,
                    "stop": {"perf/ram_util_percent":85.0}, # {"training_iteration":300},
                    "checkpoint_at_end":True,
                    "local_dir": directory,
                    "restore": start,
                    "config": {
                        "env_config": env_config,
                        "num_gpus_per_worker": 0.5,
                        "num_cpus_per_worker":4,
                        "buffer_size": 1000,
                        "num_workers": 1,
                        "model": {
                            'dim': 300,
                            'conv_filters': [
                                [16, [8, 8], 4],
                                [32, [4, 4], 2],
                                [32, [4, 4], 2],
                                [64, [19, 19], 1],
                            ],
                        },
                    },
                },
            },
            resume = False,
            )
            ray.shutdown()
            gc.collect()

    finally:
        kill_server()
        ray.shutdown()

if __name__ == "__main__":
    # If first run: this and make sure this folder is either empty or non-existing
    # If no first run: make sure that the folder only has one DQN_CarlaEnv_<xxxxx> entry
    directory = os.path.expanduser("~")+"/ray_results/a_local_dir"
    name = "dqn"
    model = "DQN"
    gc.enable()
    first_run(directory, name, model)
    restore_run(directory, name, model)