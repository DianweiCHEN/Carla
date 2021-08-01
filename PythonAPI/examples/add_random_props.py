#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Spawn random obstacles at spawn points"""

import glob
import os
import sys
import time

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse
import logging
import random

def check(a, b):
    return (abs(a-b) < 0.009999)

def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='static.prop.*',
        help='prop filter (default: "static.prop.*")')
    argparser.add_argument(
        '--height',
        metavar='h',
        default=0.0,
        type=float,
        help='move that meters all spawn points in z axis')
    argparser.add_argument(
        '--per',
        metavar='h',
        default=0.5,
        type=float,
        help='percentage to spawn')
    args = argparser.parse_args()

    vehicles_list = []
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    
    try:
        # world
        world = client.get_world()
        props = world.get_blueprint_library().filter(args.filter)

        # spawnpoints (skip the first and the last)
        spawn_points = world.get_map().get_spawn_points()
        # spawn_points = world.get_map().get_spawn_points()[1:-1]

        # move up a bit all (optional)
        for i, transform in enumerate(spawn_points):
            transform.location.z += args.height

        # spawn all kind of obstacles
        batch = []
        for transform in spawn_points:
            if (random.random() <= args.per):
                batch.append(carla.command.SpawnActor(random.choice(props), transform)
                  .then(carla.command.SetSimulatePhysics(carla.command.FutureActor, True)))
        client.apply_batch(batch)
        
    finally:
        pass

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
