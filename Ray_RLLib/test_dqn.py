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

def tune_run():
    print("THIS EXPERIMENT HAS NOT BEEN FULLY TESTED")
    gc.enable()
    directory = "/home/jacopobartiromo/ray_results/a_local_dir" #change this and make sure this folder is either empty or non-existing
    to_ignore = []
    try:
        kill_server()
        tf.keras.backend.clear_session()
        ray.init(object_store_memory=6000*1024*1024)
        run_experiments({
            "dqn": {
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
                },
            },
        },
        resume = False,
        )
        ray.shutdown()
        gc.collect()
        while (True):
            kill_server()
            tf.keras.backend.clear_session()
            ### Finds checkpoint dir path
            start = directory+"/dqn"
            for f in os.listdir(start):
                time.sleep(1)
                if "DQN" in f and not(f in to_ignore):
                    start+=("/"+f)
                    to_ignore.append(start)
                    break
            for f in os.listdir(start):
                time.sleep(1)
                if "checkpoint" in f:
                    start+=("/"+f+"/checkpoint-"+f[-1])
                    break
            ###
            ray.init(object_store_memory=6000*1024*1024)
            run_experiments({
                "dqn": {
                    "run": "DQN",
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
    tune_run()