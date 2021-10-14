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

def get_actor_blueprints(world, filter, generation):
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed
    # and therefore, we ignore the generation
    if len(bps) == 1:
        return bps

    try:
        int_generation = int(generation)
        # Check if generation is in available generations
        if int_generation in [1, 2]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []


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
        default='vehicle.*',
        help='prop filter (default: "vehicle.*")')
    argparser.add_argument(
        '--height',
        metavar='h',
        default=0.1,
        type=float,
        help='move that meters all spawn points in z axis')
    argparser.add_argument(
        '--per',
        metavar='h',
        default=1.0,
        type=float,
        help='percentage to spawn')
    argparser.add_argument(
        '--generation',
        metavar='G',
        default='All',
        help='restrict to certain vehicle generation (values: "1","2","All" - default: "All")')
    argparser.add_argument(
        '--safe',
        action='store_true',
        help='Avoid spawning vehicles prone to accidents')
    args = argparser.parse_args()

    vehicles_list = []
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    
    try:
        # world
        world = client.get_world()
        vehicles_bp = get_actor_blueprints(world, args.filter, args.generation)

        # safe vehicles (remove the AudiRC and 2 wheel vehicles)
        if args.safe:
            vehicles_bp = [x for x in vehicles_bp if int(x.get_attribute('number_of_wheels')) == 4]
            vehicles_bp = [x for x in vehicles_bp if not x.id.endswith('audirc')]

        # parked spawn points
        park_points = world.get_map().get_parked_spawn_points()

        # move up a bit all (optional)
        for i, transform in enumerate(park_points):
            transform.location.z += args.height

        # spawn all vehicles from selected blueprints
        batch = []
        total = 0
        for transform in park_points:
            if (random.random() <= args.per):
                total += 1
                batch.append(carla.command.SpawnActor(random.choice(vehicles_bp), transform)
                .then(carla.command.SetEnableGravity(carla.command.FutureActor, True)))
                # .then(carla.command.SetSimulatePhysics(carla.command.FutureActor, False)))
        client.apply_batch(batch)
        print("Adding %d parked vehicles" % total)
        
    finally:
        pass

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
