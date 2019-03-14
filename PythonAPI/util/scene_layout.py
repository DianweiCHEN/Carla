


# !/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# Provides map data for users.

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../**/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
from carla import TrafficLightState as tls

import argparse
import logging
import datetime
import weakref
import math
import random
import time


# ==============================================================================
# -- Util -----------------------------------------------------------
# ==============================================================================


class Util(object):

    @staticmethod
    def get_bounding_box(actor, actor_transform):
        bb = actor.bounding_box.extent
        corners = [
            carla.Location(x=-bb.x, y=-bb.y),
            carla.Location(x=bb.x, y=-bb.y),
            carla.Location(x=bb.x, y=bb.y),
            carla.Location(x=-bb.x, y=bb.y)]

        actor_transform.transform(corners)
        return corners

    @staticmethod
    def get_trigger_volume(actor):
        bb = actor.trigger_volume.extent
        corners = [carla.Location(x=-bb.x, y=-bb.y),
                   carla.Location(x=bb.x, y=-bb.y),
                   carla.Location(x=bb.x, y=bb.y),
                   carla.Location(x=-bb.x, y=bb.y),
                   carla.Location(x=-bb.x, y=-bb.y)]
        corners = [x + actor.trigger_volume.location for x in corners]
        t = actor.get_transform()
        t.transform(corners)
        return corners


# ==============================================================================
# -- MapData -------------------------------------------------------------------
# ==============================================================================
class MapData(object):
    def __init__(self):
        # Static Data
        self.waypoints_graph = dict()
        self.stop_signals = dict()

        # Dynamic Data
        self.hero_vehicle = []
        self.vehicles = dict()
        self.traffic_lights = dict()
        self.speed_limits = dict()
        self.walkers = dict()

    # Static data update functions
    def set_stop_signals(self, stop_signals):
        self.stop_signals.clear()
        for stop_signal in stop_signals:
            st = stop_signal.get_transform()
            stop_signal_dict = {
                "id": stop_signal.id,
                "position": [st.location.x, st.location.y, st.location.z],
                "trigger_volume": [[v.x, v.y, v.z] for v in Util.get_trigger_volume(stop_signal)]
            }
            self.stop_signals[stop_signal.id] = stop_signal_dict

    # Dynamic data update functions
    def update_hero_vehicle(self, carla_map, hero, hero_transform):
        hero_waypoint = carla_map.get_waypoint(hero_transform.location)
        hero_vehicle_dict = {
            "id": hero.id,
            "road_id": hero_waypoint.road_id,
            "lane_id": hero_waypoint.lane_id
        }
        self.hero_vehicle = hero_vehicle_dict

    def update_vehicles(self, vehicles):
        self.vehicles.clear()
        for vehicle in vehicles:
            vehicle_dict = {
                "id": vehicle[0].id,
                "position": [vehicle[1].location.x, vehicle[1].location.y, vehicle[1].location.z],
                "orientation": [vehicle[1].rotation.roll, vehicle[1].rotation.pitch,
                                vehicle[1].rotation.yaw],
                "bounding_box": [[v.x, v.y, v.z] for v in
                                 Util.get_bounding_box(vehicle[0], vehicle[1])]
            }
            self.vehicles[vehicle[0].id] = vehicle_dict

    def update_traffic_lights(self, traffic_lights):
        self.traffic_lights.clear()
        for traffic_light in traffic_lights:
            traffic_light_dict = {
                "id": traffic_light[0].id,
                "state": traffic_light[0].state,
                "position": [traffic_light[1].location.x, traffic_light[1].location.y,
                             traffic_light[1].location.z],
                "trigger_volume": [[v.x, v.y, v.z] for v in
                                   Util.get_trigger_volume(traffic_light[0])]
            }
            self.traffic_lights[traffic_light[0].id] = traffic_light_dict

    def update_speed_limits(self, speed_limits):
        self.speed_limits.clear()
        for speed_limit in speed_limits:
            speed_limit_dict = {
                "id": speed_limit[0].id,
                "position": [speed_limit[1].location.x, speed_limit[1].location.y,
                             speed_limit[1].location.z],
                "speed": int(speed_limit[0].type_id.split('.')[2])
            }
            self.speed_limits[speed_limit[0].id] = speed_limit_dict

    def update_walkers(self, walkers):
        self.walkers.clear()
        for walker in walkers:
            walker_dict = {
                "id": walker[0].id,
                "position": [walker[1].location.x, walker[1].location.y, walker[1].location.z],
                "orientation": [walker[1].rotation.roll, walker[1].rotation.pitch,
                                walker[1].rotation.yaw],
                "bounding_box": [[v.x, v.y, v.z] for v in
                                 Util.get_bounding_box(walker[0], walker[1])]
            }
            self.walkers[walker[0].id] = walker_dict


map_data = MapData()


def get_scene_layout(world,map):

    """
    Function to extract the full scene layout to be used as a full scene description to be
    given to the user
    :param world: the world object from CARLA
    :return: a dictionary describing the scene.
    """

    ######## PARSE HERE THE MAP WITH THE NECESSARY FUNCTIONS. REUSE NON RENDERING MODE


    return {

        1: {
            "road_id": 0,
            "lane_id": 0,
            "position": [0, 0, 0],  # vector 3D
            "orientation": [0, 0, 0],   ## Roll, Pitch Yaw
            "left_margin_gps_and_z": [0.3, 0.1, 23],
            "right_margin_gps_and_z": [0.1, 0.01, 23],
            "next_waypoints_ids": [2],
            "left_lane_waypoint_id": -1,
            "right_lane_waypoint_id": -1
        },
        2: {
            "road_id": 0,
            "lane_id": 0,
            "position": [0, 0, 0],  # vector 3D
            "orientation": [0, 0, 0],   ## Roll, Pitch Yaw
            "left_margin_gps_and_z": [0.3, 0.1, 23],
            "right_margin_gps_and_z": [0.1, 0.01, 23],
            "next_waypoints_ids": [4, 5],
            "left_lane_waypoint_id": -1,
            "right_lane_waypoint_id": -1
        }

    }


def get_dynamic_objects(world,map):


    def get_stop_signals(stopsignals):

        return [ {
                    "id": 99,
                    "position": [0,0,0], # x,y,z
                    "trigger_volume": [[],[]] # Set of points

                }

        ]

    def get_traffic_lights(traffic_lights):
        return [{
            "id": 99,
            "state": 0,  # Red yellow grean
            "position": [0, 0, 0],  # x,y,z
            "trigger_volume": [[], []]  # Set of points

        }]

    def get_vehicles(vehicles):


         return [{
             "id": 99,
              "position": [0,0,0], #x,y,z
              "orientation": [0,0,0], # roll pitch yaw
              "bounding_box": [[]] # set of points
            }
         ]

    def get_walkers(walkers):

        return [{
            "id": 99,
            "position": [0, 0, 0],  # x,y,z
            "orientation": [0, 0, 0],  # roll pitch yaw
            "bounding_box": [[]]  # set of points
        }]

    def get_speed_limits(speed_limits):



        return  [{
                "id": 99,
                "position": [0, 0, 0],  # x,y,z
                "speed": 90  # Set of points

            }]




    return {
        'vehicles': get_vehicles(None),  # add the appropriate values baserd on world
        'walkers': get_walkers(None),
        'traffic_lights': get_traffic_lights(None),
        'stop_signs': get_stop_signals(None),
        'speed_limits': get_speed_limits(None)

    }


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================


class World(object):
    def __init__(self, host, port, timeout):
        self.world, self.town_map = self._get_data_from_carla(host, port, timeout)
        self.hero_vehicle = None
        print (self.world)
        weak_self = weakref.ref(self)
        self.world.on_tick(lambda timestamp: World.on_world_tick(weak_self, timestamp))

    def _get_data_from_carla(self, host, port, timeout):
        try:
            self.client = carla.Client(host, port)
            self.client.set_timeout(timeout)

            world = self.client.get_world()
            town_map = world.get_map()
            return (world, town_map)

        except Exception as ex:
            logging.error(ex)

    def _split_actors(self):
        vehicles = []
        traffic_lights = []
        speed_limits = []
        walkers = []

        for actor_with_transform in self.actors_with_transforms:
            actor = actor_with_transform[0]
            if 'vehicle' in actor.type_id:
                vehicles.append(actor_with_transform)
            elif 'traffic_light' in actor.type_id:
                traffic_lights.append(actor_with_transform)
            elif 'speed_limit' in actor.type_id:
                speed_limits.append(actor_with_transform)
            elif 'walker' in actor.type_id:
                walkers.append(actor_with_transform)

        return (vehicles, traffic_lights, speed_limits, walkers)

    @staticmethod
    def on_world_tick(weak_self, timestamp):
        self = weak_self()
        if not self:
            return

        self.server_clock.tick()
        self.server_fps = self.server_clock.get_fps()
        self.simulation_time = timestamp.elapsed_seconds

        vehicles, traffic_lights, speed_limits, walkers = self._split_actors()
        hero_vehicles = [vehicle for vehicle in vehicles if
                         'vehicle' in vehicle.type_id and vehicle.attributes['role_name'] == 'hero']

        if len(hero_vehicles) > 0:
            self.hero_vehicle = random.choice(hero_vehicles)

        if self.hero_vehicle is not None:
            map_data.update_hero_vehicle(self.town_map, self.hero_vehicle, self.hero_transform)

        map_data.update_vehicles(vehicles)
        map_data.update_traffic_lights(traffic_lights)
        map_data.update_speed_limits(speed_limits)
        map_data.update_walkers(walkers)

    def tick(self):
        actors = self.world.get_actors()
        self.actors_with_transforms = [(actor, actor.get_transform()) for actor in actors]


world = None


def connect(args):
    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)

    world = World(args.host, args.port, 2.0)


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================

def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Map Data Extractor')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
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
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    connect(args)

    while True:
        if world is not None:
            world.tick()

    print(__doc__)


if __name__ == '__main__':
    main()