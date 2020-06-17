#!/usr/bin/env python

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import time
import argparse


def evaluate_sync(world, test_time):
    print('Ignoring the first 100 frames...')
    for _ in range(100):
        world.tick()
    print(f'Starting {test_time}s of sync test...')
    ticks = 0
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < test_time:
        world.tick()
        ticks += 1
    print("Made", ticks / test_time, "ticks per second")

def evaluate_async(world, test_time):
    print('Ignoring the first 2 seconds...')
    time.sleep(2)
    print(f'Starting {test_time}s of async test...')
    ticks = 0
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < test_time:
        world.wait_for_tick(2.0) # timeout = 2 sec
        ticks += 1
    print("Made", ticks / test_time, "ticks per second")

def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '-s', '--sync',
        action='store_true',
        help='synchronous mode')
    argparser.add_argument(
        '-m', '--map',
        default='Town03',
        help='load specific CARLA map')
    argparser.add_argument(
        '-t', '--test-time',
        default=1.0,
        type=float,
        help='total time to benchmark')

    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)

    world = client.load_world(args.map)

    original_settings = world.get_settings()
    settings = world.get_settings()

    try:
        if args.sync:
            settings.fixed_delta_seconds = 0.05
            settings.synchronous_mode = True
        else:
            settings.fixed_delta_seconds = 0.0
            settings.synchronous_mode = False
        world.apply_settings(settings)

        # make first tick
        if args.sync:
            world.tick()
        else:
            world.wait_for_tick()

        # evaluate tick performance per second
        if args.sync:
            evaluate_sync(world, args.test_time)
        else:
            evaluate_async(world, args.test_time)

    finally:
        world.apply_settings(original_settings)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user.')
    except RuntimeError as e:
        print(e)
