"""Ape-X DQN Algorithm. Tested with CARLA.
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

os.environ['TF_FORCE_GPU_ALLOW_GROWTH'] = 'true'
from tensorflow.compat.v1.keras.backend import set_session

config = tf.compat.v1.ConfigProto()

config.gpu_options.allow_growth = True  # dynamically grow the memory used on the GPU

config.log_device_placement = True  # to log device placement (on which device the operation ran)

sess = tf.compat.v1.Session(config=config)

set_session(sess)
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
    ray.init()
    run_experiments({
        "apex-vision": {
            "run": "APEX",
            "env": CarlaEnv,
            "stop":{"episodes_total":30000000},#"training_iteration":5000000},
            "checkpoint_at_end":True,
            "checkpoint_freq":100,
            "config": {
                "env_config": env_config,
                "num_gpus_per_worker": 0,
                "num_cpus_per_worker":2,
                "buffer_size":20000,
                "num_workers": 1,
            },
        },
    },
    resume= False,
    )