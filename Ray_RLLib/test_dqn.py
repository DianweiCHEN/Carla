"""DQN Algorithm. Tested with CARLA.
You can visualize experiment results in ~/ray_results using TensorBoard.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import os
import shutil
import pickle

import ray
from ray import tune
from ray.rllib.agents.dqn import DQNTrainer
import torch

from carla_env import CarlaEnv
from helper.CarlaHelper import kill_server


class CustomDQNTrainer(DQNTrainer):
    def save_checkpoint(self, checkpoint_dir):
        checkpoint_path = super().save_checkpoint(checkpoint_dir)

        model = self.get_policy().model
        torch.save({
            "model_state_dict": model.state_dict(),
        }, os.path.join(checkpoint_dir, "checkpoint.pth"))

        return checkpoint_path


env_config = {
    "RAY": True,  # Are we running an experiment in Ray
    "DEBUG_MODE": False,
    "Experiment": "experiment2",
}


def find_latest_checkpoint(args):
    """
    Finds the latest checkpoint, based on how RLLib creates and names them.
    """
    start = args.directory + "/" + args.name
    max_f = ""
    max_g = ""
    max_checkpoint = 0
    for f in os.listdir(start):
        if args.algorithm in f:
            temp = start + "/" + f
            for g in os.listdir(temp):
                if "checkpoint_" in g:
                    episode = int(''.join([n for n in g if n.isdigit()]))
                    if episode > max_checkpoint:
                        max_checkpoint = episode
                        max_f = f
                        max_g = g
    if max_checkpoint == 0:
        print(
            "Could not find any checkpoint, make sure that you have selected the correct folder path"
        )
        raise IndexError
    start += ("/" + max_f + "/" + max_g + "/" + max_g.replace("_", "-"))
    return start


def run(args):
    try:
        if args.restore:
            checkpoint = find_latest_checkpoint(args)
        else:
            checkpoint = False
        while True:
            kill_server()
            ray.init()
            tune.run(
                CustomDQNTrainer,
                name=args.name,
                local_dir=args.directory,
                stop={"perf/ram_util_percent": 85.0},
                checkpoint_freq=1,
                checkpoint_at_end=True,
                restore=checkpoint,
                config={
                    "log_level": "DEBUG",
                    "horizon": 2500,
                    "learning_starts": 1000,
                    "buffer_size": 50000,
                    "env": CarlaEnv,
                    "env_config": env_config,
                    "framework": "torch",
                    "num_gpus_per_worker": 0.2,
                    "num_cpus_per_worker": 2,
                    "num_workers": 20,
                    "timesteps_per_iteration": 5000,
                    "exploration_config": {
                        "type": "EpsilonGreedy",
                        "initial_epsilon": 1.0,
                        "final_epsilon": 0.02,
                        "epsilon_timesteps": 1000000
                    },
                    "model": {
                        'dim': 190,
                        'conv_filters': [
                            [16, [7, 7], 2],
                            [16, [7, 7], 1],
                            [32, [7, 7], 2],
                            [32, [7, 7], 1],
                            [64, [7, 7], 2],
                            [64, [7, 7], 1],
                            [128, [7, 7], 2],
                            [128, [7, 7], 2],
                            [256, [6, 6], 1],
                        ],
                    },
                },
            )
            ray.shutdown()
            checkpoint = find_latest_checkpoint(args)

    finally:
        kill_server()
        ray.shutdown()


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('-d',
                           '--directory',
                           metavar='D',
                           default=os.path.expanduser("~") + "/ray_results/newdir3",
                           help='Specified directory to save results')
    argparser.add_argument('-n',
                           '--name',
                           metavar='P',
                           default="dqn",
                           help='Name of the experiment (default: dqn)')
    argparser.add_argument('-a',
                           '--algorithm',
                           metavar='P',
                           default="DQN",
                           help='Algorithm used by the experiment (default: DQN)')
    argparser.add_argument('--restore',
                           action='store_true',
                           default=False,
                           help='Flag to restore from the specified directory')
    argparser.add_argument(
        '--override',
        action='store_true',
        default=False,
        help=
        'Flag to override a specific directory (warning: all content of the folder will be lost.)')

    args = argparser.parse_args()

    #directory = args.directory + "/" + args.name
    # if not args.restore:
    # if os.path.exists(directory):
    # if args.override and os.path.isdir(directory):
    # shutil.rmtree(directory)
    # elif len(os.listdir(directory)) != 0:
    # print("The directory " + directory + " is not empty. To start a new training instance, make sure this folder is either empty or non-existing.")
    # return
    # else:
    # if not(os.path.exists(directory)) or len(os.listdir(directory)) == 0:
    # print("You can't restore from an empty or non-existing directory. To restore a training instance, make sure there is at least one checkpoint.")
    run(args)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
