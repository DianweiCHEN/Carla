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

from carla import ColorConverter as cc

def main():
    # We start creating the client
    client = carla.Client('localhost', 2000)
    client.set_timeout(5.0)
    world = client.get_world()

    weather = world.get_weather()
    light_manager = world.get_lightmanager()
    rgb_camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
    rgb_camera_actor = world.spawn_actor(rgb_camera_bp, carla.Transform(carla.Location(x=0.0, y=0.0, z=0.0)))

    connection = sqlite3.connect('carla_render.db')
    c = connection.cursor()
    c.execute('''CREATE TABLE WeatherParameters(cloudiness REAL, precipitation REAL, precipitation_deposits REAL, wind_intensity REAL, sun_azimuth_angle REAL, sun_altitude_angle REAL, fog_density REAL, fog_distance REAL, wetness REAL, fog_falloff REAL)''')
    c.execute('''CREATE TABLE StreetLights(id INT, colorR REAL, colorG REAL, colorB REAL, colorA REAL, intensity REAL, is_on BOOL, locationX REAL, locationY REAL, locationZ REAL)''')
    c.execute('''CREATE TABLE BuildingLights(id INT, colorR REAL, colorG REAL, colorB REAL, colorA REAL, intensity REAL, is_on BOOL, locationX REAL, locationY REAL, locationZ REAL)''')
    c.execute('''CREATE TABLE OtherLights(id INT, colorR REAL, colorG REAL, colorB REAL, colorA REAL, intensity REAL, is_on BOOL, locationX REAL, locationY REAL, locationZ REAL)''')
    c.execute('''CREATE TABLE RGBCamera(chromatic_aberration_offset REAL, sensor_tick REAL, fstop REAL, image_size_x REAL, image_size_y REAL, fov REAL, lens_circle_falloff REAL,
                lens_circle_multiplier REAL, exposure_compensation REAL, lens_y_size REAL, lens_k REAL, exposure_mode TEXT, lens_kcube REAL, lens_x_size REAL, exposure_max_bright REAL,
                shutter_speed REAL, bloom_intensity REAL, iso REAL, enable_postprocess_effects INT, gamma REAL, motion_blur_intensity REAL, motion_blur_max_distortion REAL,
                lens_flare_intensity REAL, motion_blur_min_object_screen_size REAL, exposure_min_bright REAL, exposure_speed_up REAL, exposure_speed_down REAL, tint REAL,
                calibration_constant REAL, focal_distance REAL, min_fstop REAL, blade_count INT, blur_amount REAL, blur_radius REAL, slope REAL, toe REAL, white_clip REAL, shoulder REAL,
                black_clip REAL, temp REAL, chromatic_aberration_intensity REAL)''')
    
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
    print("Saved carla.LightGroup.Other data")

    c.execute('''INSERT INTO RGBCamera VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)''', (rgb_camera_actor.attributes["chromatic_aberration_offset"], rgb_camera_actor.attributes["sensor_tick"],
    rgb_camera_actor.attributes["fstop"], rgb_camera_actor.attributes["image_size_x"], rgb_camera_actor.attributes["image_size_y"], rgb_camera_actor.attributes["fov"],
    rgb_camera_actor.attributes["lens_circle_falloff"], rgb_camera_actor.attributes["lens_circle_multiplier"], rgb_camera_actor.attributes["exposure_compensation"],
    rgb_camera_actor.attributes["lens_y_size"], rgb_camera_actor.attributes["lens_k"], rgb_camera_actor.attributes["exposure_mode"], rgb_camera_actor.attributes["lens_kcube"],
    rgb_camera_actor.attributes["lens_x_size"], rgb_camera_actor.attributes["exposure_max_bright"], rgb_camera_actor.attributes["shutter_speed"], rgb_camera_actor.attributes["bloom_intensity"],
    rgb_camera_actor.attributes["iso"], rgb_camera_actor.attributes["enable_postprocess_effects"], rgb_camera_actor.attributes["gamma"], rgb_camera_actor.attributes["motion_blur_intensity"],
    rgb_camera_actor.attributes["motion_blur_max_distortion"], rgb_camera_actor.attributes["lens_flare_intensity"], rgb_camera_actor.attributes["motion_blur_min_object_screen_size"],
    rgb_camera_actor.attributes["exposure_min_bright"], rgb_camera_actor.attributes["exposure_speed_up"], rgb_camera_actor.attributes["exposure_speed_down"], rgb_camera_actor.attributes["tint"],
    rgb_camera_actor.attributes["calibration_constant"], rgb_camera_actor.attributes["focal_distance"],  rgb_camera_actor.attributes["min_fstop"], rgb_camera_actor.attributes["blade_count"],
    rgb_camera_actor.attributes["blur_amount"], rgb_camera_actor.attributes["blur_radius"], rgb_camera_actor.attributes["slope"], rgb_camera_actor.attributes["toe"],
    rgb_camera_actor.attributes["white_clip"], rgb_camera_actor.attributes["shoulder"], rgb_camera_actor.attributes["black_clip"], rgb_camera_actor.attributes["temp"],
    rgb_camera_actor.attributes["chromatic_aberration_intensity"]))
    print("Saved carla.RGBCamera data\n")

    connection.commit()
    print("Generated render database succesfully!")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')