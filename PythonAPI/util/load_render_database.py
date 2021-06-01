#!/usr/bin/env python

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import glob
import os
import sys
import sqlite3

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

def main():
    # We start creating the client
    client = carla.Client('localhost', 2000)
    client.set_timeout(5.0)
    world = client.get_world()

    weather = world.get_weather()
    light_manager = world.get_lightmanager()

    connection = sqlite3.connect('carla_render.db')
    c = connection.cursor()
    c.execute('''CREATE TABLE WeatherParameters(cloudiness REAL, precipitation REAL, precipitation_deposits REAL, wind_intensity REAL, sun_azimuth_angle REAL, sun_altitude_angle REAL, fog_density REAL, fog_distance REAL, wetness REAL, fog_falloff REAL)''')
    c.execute('''CREATE TABLE StreetLights(id INT, colorR REAL, colorG REAL, colorB REAL, colorA REAL, intensity REAL, is_on BOOL, locationX REAL, locationY REAL, locationZ REAL)''')
    c.execute('''CREATE TABLE BuildingLights(id INT, colorR REAL, colorG REAL, colorB REAL, colorA REAL, intensity REAL, is_on BOOL, locationX REAL, locationY REAL, locationZ REAL)''')
    c.execute('''CREATE TABLE OtherLights(id INT, colorR REAL, colorG REAL, colorB REAL, colorA REAL, intensity REAL, is_on BOOL, locationX REAL, locationY REAL, locationZ REAL)''')

    #Store weather parameters
    c.execute('''INSERT INTO WeatherParameters VALUES(?,?,?,?,?,?,?,?,?,?)''', (weather.cloudiness,
    weather.precipitation, weather.precipitation_deposits, weather.wind_intensity, weather.sun_azimuth_angle,
    weather.sun_altitude_angle, weather.fog_density, weather.fog_distance, weather.wetness, weather.fog_falloff))
    print("Saved carla.WeatherParameters data")

    #Store light parameters
    lightlist_street = light_manager.get_all_lights(carla.LightGroup.Street)
    lightlist_building = light_manager.get_all_lights(carla.LightGroup.Building)
    lightlist_other = light_manager.get_all_lights(carla.LightGroup.Other)

    for light in lightlist_street:
        c.execute('''INSERT INTO StreetLights VALUES(?,?,?,?,?,?,?,?,?,?)''', (light.id,
        light.color.r, light.color.g, light.color.b, light.color.a, light.intensity, light.is_on, light.location.x, light.location.y, light.location.z))
    print("Saved carla.LightGroup.Street data")

    for light in lightlist_building:
        c.execute('''INSERT INTO BuildingLights VALUES(?,?,?,?,?,?,?,?,?,?)''', (light.id,
        light.color.r, light.color.g, light.color.b, light.color.a, light.intensity, light.is_on, light.location.x, light.location.y, light.location.z))
    print("Saved carla.LightGroup.Building data")

    for light in lightlist_other:
        c.execute('''INSERT INTO OtherLights VALUES(?,?,?,?,?,?,?,?,?,?)''', (light.id,
        light.color.r, light.color.g, light.color.b, light.color.a, light.intensity, light.is_on, light.location.x, light.location.y, light.location.z))
    print("Saved carla.LightGroup.Other data\n")

    connection.commit()
    print("Generated render database succesfully!")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')