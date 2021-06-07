#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
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

    print("Loading render database...")
    connection = sqlite3.connect('carla_render.db')
    c = connection.cursor()

    weather = world.get_weather()
    light_manager = world.get_lightmanager()
    lightlist_street = light_manager.get_all_lights(carla.LightGroup.Street)
    lightlist_building = light_manager.get_all_lights(carla.LightGroup.Building)
    lightlist_other = light_manager.get_all_lights(carla.LightGroup.Other)
    rgb_camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
    rgb_camera_actor = world.spawn_actor(rgb_camera_bp, carla.Transform(carla.Location(x=0.0, y=0.0, z=0.0)))

    #Compare weather parameters with simulation
    print("Comparing WEATHER PARAMETERS data with simulation\n\n")
    db_data = c.execute('''SELECT * FROM WeatherParameters''')
    db_data_list = []
    for value in db_data:
        db_data_list.append(value)
    if (weather.cloudiness != db_data_list[0][0]):
        print("[WEATHER MISMATCH] Cloudiness -> [World Data] " + str(weather.cloudiness) + " [DB Data] : " + str(db_data_list[0][0]))
    if (weather.precipitation != db_data_list[0][1]):
        print("[WEATHER MISMATCH] Precipitation -> [World Data] " + str(weather.precipitation) + " [DB Data] : " + str(db_data_list[0][1]))
    if (weather.precipitation_deposits != db_data_list[0][2]):
        print("[WEATHER MISMATCH] Precipitation Deposits -> [World Data] " + str(weather.precipitation_deposits) + " [DB Data] : " + str(db_data_list[0][2]))
    if (weather.wind_intensity != db_data_list[0][3]):
        print("[WEATHER MISMATCH] Wind Intensity -> [World Data] " + str(weather.wind_intensity) + " [DB Data] : " + str(db_data_list[0][3]))
    if (weather.sun_azimuth_angle != db_data_list[0][4]):
        print("[WEATHER MISMATCH] Sun Azimuth Angle -> [World Data] " + str(weather.sun_azimuth_angle) + " [DB Data] : " + str(db_data_list[0][4]))
    if (weather.sun_altitude_angle != db_data_list[0][5]):
        print("[WEATHER MISMATCH] Sun Altitude Angle -> [World Data] " + str(weather.sun_altitude_angle) + " [DB Data] : " + str(db_data_list[0][5]))
    if (weather.fog_density != db_data_list[0][6]):
        print("[WEATHER MISMATCH] Fog Density -> [World Data] " + str(weather.fog_density) + " [DB Data] : " + str(db_data_list[0][6]))
    if (weather.fog_distance != db_data_list[0][7]):
        print("[WEATHER MISMATCH] Fog Distance -> [World Data] " + str(weather.fog_distance) + " [DB Data] : " + str(db_data_list[0][7]))
    if (weather.wetness != db_data_list[0][8]):
        print("[WEATHER MISMATCH] Wetness -> [World Data] " + str(weather.wetness) + " [DB Data] : " + str(db_data_list[0][8]))
    if (weather.fog_falloff != db_data_list[0][9]):
        print("[WEATHER MISMATCH] Fog Falloff -> [World Data] " + str(weather.fog_falloff) + " [DB Data] : " + str(db_data_list[0][9]))

    #Check street lights
    print("Comparing STREET LIGHTS data with simulation\n\n")
    for _i in range(0, len(lightlist_street)) :
        db_data = c.execute('''SELECT * FROM StreetLights WHERE id = ''' + str(lightlist_street[_i].id)).fetchall()
        db_data_list = []
        for value in db_data:
            db_data_list.append(value) 
        if(lightlist_street[_i].color.r != db_data_list[0][1]):
            print("[STREET LIGHTS MISMATCH] Color.R | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].color.r) + " [DB Data] : " + str(db_data_list[0][1]))
        if(lightlist_street[_i].color.g != db_data_list[0][2]):
            print("[STREET LIGHTS MISMATCH] Color.G | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].color.g) + " [DB Data] : " + str(db_data_list[0][2]))
        if(lightlist_street[_i].color.b != db_data_list[0][3]):
            print("[STREET LIGHTS MISMATCH] Color.B | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].color.b) + " [DB Data] : " + str(db_data_list[0][3]))
        if(lightlist_street[_i].color.a != db_data_list[0][4]):
            print("[STREET LIGHTS MISMATCH] Color.A | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].color.a) + " [DB Data] : " + str(db_data_list[0][4]))
        if(lightlist_street[_i].intensity != db_data_list[0][5]):
            print("[STREET LIGHTS MISMATCH] Intensity | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].intensity) + " [DB Data] : " + str(db_data_list[0][5]))
        if(lightlist_street[_i].is_on != db_data_list[0][6]):
            print("[STREET LIGHTS MISMATCH] Is on | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].is_on) + " [DB Data] : " + str(db_data_list[0][6]))
        if(lightlist_street[_i].location.x != db_data_list[0][7]):
            print("[STREET LIGHTS MISMATCH] Location X | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].location.x) + " [DB Data] : " + str(db_data_list[0][7]))
        if(lightlist_street[_i].location.y != db_data_list[0][8]):
            print("[STREET LIGHTS MISMATCH] Location Y | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].location.y) + " [DB Data] : " + str(db_data_list[0][8]))
        if(lightlist_street[_i].location.z != db_data_list[0][9]):
            print("[STREET LIGHTS MISMATCH] Location Z | ID -> : " + str(lightlist_street[_i].id) + " | [World Data] " + str(lightlist_street[_i].location.z) + " [DB Data] : " + str(db_data_list[0][9]))

    #Check building lights
    print("Comparing BUILDING LIGHTS data with simulation\n\n")
    for _i in range(0, len(lightlist_building)) :
        db_data = c.execute('''SELECT * FROM BuildingLights WHERE id = ''' + str(lightlist_building[_i].id)).fetchall()
        db_data_list = []
        for value in db_data :
            db_data_list.append(value) 
        if(lightlist_building[_i].color.r != db_data_list[0][1]):
            print("[BUILDING LIGHTS MISMATCH] Color.R | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].color.r) + " [DB Data] : " + str(db_data_list[0][1]))
        if(lightlist_building[_i].color.g != db_data_list[0][2]):
            print("[BUILDING LIGHTS MISMATCH] Color.G | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].color.g) + " [DB Data] : " + str(db_data_list[0][2]))
        if(lightlist_building[_i].color.b != db_data_list[0][3]):
            print("[BUILDING LIGHTS MISMATCH] Color.B | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].color.b) + " [DB Data] : " + str(db_data_list[0][3]))
        if(lightlist_building[_i].color.a != db_data_list[0][4]):
            print("[BUILDING LIGHTS MISMATCH] Color.A | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].color.a) + " [DB Data] : " + str(db_data_list[0][4]))
        if(lightlist_building[_i].intensity != db_data_list[0][5]):
            print("[BUILDING LIGHTS MISMATCH] Intensity | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].intensity) + " [DB Data] : " + str(db_data_list[0][5]))
        if(lightlist_building[_i].is_on != db_data_list[0][6]):
            print("[BUILDING LIGHTS MISMATCH] Is on | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].is_on) + " [DB Data] : " + str(db_data_list[0][6]))
        if(lightlist_building[_i].location.x != db_data_list[0][7]):
            print("[BUILDING LIGHTS MISMATCH] Location X | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].location.x) + " [DB Data] : " + str(db_data_list[0][7]))
        if(lightlist_building[_i].location.y != db_data_list[0][8]):
            print("[BUILDING LIGHTS MISMATCH] Location Y | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].location.y) + " [DB Data] : " + str(db_data_list[0][8]))
        if(lightlist_building[_i].location.z != db_data_list[0][9]):
            print("[BUILDING LIGHTS MISMATCH] Location Z | ID -> : " + str(lightlist_building[_i].id) + " | [World Data] " + str(lightlist_building[_i].location.z) + " [DB Data] : " + str(db_data_list[0][9]))

    #Check other lights
    print("Comparing OTHER LIGHTS data with simulation\n\n")
    for _i in range(0, len(lightlist_other)) :
        db_data = c.execute('''SELECT * FROM OtherLights WHERE id = ''' + str(lightlist_other[_i].id)).fetchall()
        db_data_list = []
        for value in db_data :
            db_data_list.append(value) 
        if(lightlist_other[_i].color.r != db_data_list[0][1]):
            print("[OTHER LIGHTS MISMATCH] Color.R | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].color.r) + " [DB Data] : " + str(db_data_list[0][1]))
        if(lightlist_other[_i].color.g != db_data_list[0][2]):
            print("[OTHER LIGHTS MISMATCH] Color.G | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].color.g) + " [DB Data] : " + str(db_data_list[0][2]))
        if(lightlist_other[_i].color.b != db_data_list[0][3]):
            print("[OTHER LIGHTS MISMATCH] Color.B | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].color.b) + " [DB Data] : " + str(db_data_list[0][3]))
        if(lightlist_other[_i].color.a != db_data_list[0][4]):
            print("[OTHER LIGHTS MISMATCH] Color.A | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].color.a) + " [DB Data] : " + str(db_data_list[0][4]))
        if(lightlist_other[_i].intensity != db_data_list[0][5]):
            print("[OTHER LIGHTS MISMATCH] Intensity | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].intensity) + " [DB Data] : " + str(db_data_list[0][5]))
        if(lightlist_other[_i].is_on != db_data_list[0][6]):
            print("[OTHER LIGHTS MISMATCH] Is on | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].is_on) + " [DB Data] : " + str(db_data_list[0][6]))
        if(lightlist_other[_i].location.x != db_data_list[0][7]):
            print("[OTHER LIGHTS MISMATCH] Location X | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].location.x) + " [DB Data] : " + str(db_data_list[0][7]))
        if(lightlist_other[_i].location.y != db_data_list[0][8]):
            print("[OTHER LIGHTS MISMATCH] Location Y | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].location.y) + " [DB Data] : " + str(db_data_list[0][8]))
        if(lightlist_other[_i].location.z != db_data_list[0][9]):
            print("[OTHER LIGHTS MISMATCH] Location Z | ID -> : " + str(lightlist_other[_i].id) + " | [World Data] " + str(lightlist_other[_i].location.z) + " [DB Data] : " + str(db_data_list[0][9]))

    #Compare RGB camera data with simulation
    print("Comparing RGBCamera PARAMETERS data with simulation\n\n")
    db_data = c.execute('''SELECT * FROM RGBCamera''')
    db_data_list = []
    for value in db_data:
        db_data_list.append(value)
    if (float(rgb_camera_actor.attributes["chromatic_aberration_offset"]) != db_data_list[0][0]):
        print("[RGB PARAMETER MISMATCH] Chromatic Aberration Offset -> [World Data] " + str(rgb_camera_actor.attributes["chromatic_aberration_offset"]) + " [DB Data] : " + str(db_data_list[0][0]))
    if (float(rgb_camera_actor.attributes["sensor_tick"]) != db_data_list[0][1]):
        print("[RGB PARAMETER MISMATCH] Sensor tick -> [World Data] " + str(rgb_camera_actor.attributes["sensor_tick"]) + " [DB Data] : " + str(db_data_list[0][1]))
    if (float(rgb_camera_actor.attributes["fstop"]) != db_data_list[0][2]):
        print("[RGB PARAMETER MISMATCH] Fstop -> [World Data] " + str(rgb_camera_actor.attributes["fstop"]) + " [DB Data] : " + str(db_data_list[0][2]))
    if (float(rgb_camera_actor.attributes["image_size_x"]) != db_data_list[0][3]):
        print("[RGB PARAMETER MISMATCH] Image Size X -> [World Data] " + str(rgb_camera_actor.attributes["image_size_x"]) + " [DB Data] : " + str(db_data_list[0][3]))
    if (float(rgb_camera_actor.attributes["image_size_y"]) != db_data_list[0][4]):
        print("[RGB PARAMETER MISMATCH] Image Size Y -> [World Data] " + str(rgb_camera_actor.attributes["image_size_y"]) + " [DB Data] : " + str(db_data_list[0][4]))
    if (float(rgb_camera_actor.attributes["fov"]) != db_data_list[0][5]):
        print("[RGB PARAMETER MISMATCH] Fov -> [World Data] " + str(rgb_camera_actor.attributes["fov"]) + " [DB Data] : " + str(db_data_list[0][5]))
    if (float(rgb_camera_actor.attributes["lens_circle_falloff"]) != db_data_list[0][6]):
        print("[RGB PARAMETER MISMATCH] Lens circle falloff -> [World Data] " + str(rgb_camera_actor.attributes["lens_circle_falloff"]) + " [DB Data] : " + str(db_data_list[0][6]))
    if (float(rgb_camera_actor.attributes["lens_circle_multiplier"]) != db_data_list[0][7]):
        print("[RGB PARAMETER MISMATCH] Lens circle multiplier -> [World Data] " + str(rgb_camera_actor.attributes["lens_circle_multiplier"]) + " [DB Data] : " + str(db_data_list[0][7]))
    if (float(rgb_camera_actor.attributes["exposure_compensation"]) != db_data_list[0][8]):
        print("[RGB PARAMETER MISMATCH] Exposure compensation -> [World Data] " + str(rgb_camera_actor.attributes["exposure_compensation"]) + " [DB Data] : " + str(db_data_list[0][8]))
    if (float(rgb_camera_actor.attributes["lens_y_size"]) != db_data_list[0][9]):
        print("[RGB PARAMETER MISMATCH] Lens Y Size -> [World Data] " + str(rgb_camera_actor.attributes["lens_y_size"]) + " [DB Data] : " + str(db_data_list[0][9]))
    if (float(rgb_camera_actor.attributes["lens_k"]) != db_data_list[0][10]):
        print("[RGB PARAMETER MISMATCH] LensK -> [World Data] " + str(rgb_camera_actor.attributes["lens_k"]) + " [DB Data] : " + str(db_data_list[0][10]))
    if (rgb_camera_actor.attributes["exposure_mode"] != db_data_list[0][11]): #This is a text
        print("[RGB PARAMETER MISMATCH] Exposure mode -> [World Data] " + rgb_camera_actor.attributes["exposure_mode"] + " [DB Data] : " + db_data_list[0][11])
    if (float(rgb_camera_actor.attributes["lens_kcube"]) != db_data_list[0][12]):
        print("[RGB PARAMETER MISMATCH] LensKCube -> [World Data] " + str(rgb_camera_actor.attributes["lens_kcube"]) + " [DB Data] : " + str(db_data_list[0][12]))
    if (float(rgb_camera_actor.attributes["lens_x_size"]) != db_data_list[0][13]):
        print("[RGB PARAMETER MISMATCH] Lens X Size -> [World Data] " + str(rgb_camera_actor.attributes["lens_x_size"]) + " [DB Data] : " + str(db_data_list[0][13]))
    if (float(rgb_camera_actor.attributes["exposure_max_bright"]) != db_data_list[0][14]):
        print("[RGB PARAMETER MISMATCH] Exposure max bright -> [World Data] " + str(rgb_camera_actor.attributes["exposure_max_bright"]) + " [DB Data] : " + str(db_data_list[0][14]))
    if (float(rgb_camera_actor.attributes["shutter_speed"]) != db_data_list[0][15]):
        print("[RGB PARAMETER MISMATCH] Shutter speed -> [World Data] " + str(rgb_camera_actor.attributes["shutter_speed"]) + " [DB Data] : " + str(db_data_list[0][15]))
    if (float(rgb_camera_actor.attributes["bloom_intensity"]) != db_data_list[0][16]):
        print("[RGB PARAMETER MISMATCH] Bloom intesity -> [World Data] " + str(rgb_camera_actor.attributes["bloom_intensity"]) + " [DB Data] : " + str(db_data_list[0][16]))
    if (float(rgb_camera_actor.attributes["iso"]) != db_data_list[0][17]):
        print("[RGB PARAMETER MISMATCH] Iso -> [World Data] " + str(rgb_camera_actor.attributes["iso"]) + " [DB Data] : " + str(db_data_list[0][17]))
    if (rgb_camera_actor.attributes["enable_postprocess_effects"] != db_data_list[0][18]):
        print("[RGB PARAMETER MISMATCH] Enable Post-Process effects -> [World Data] " + str(rgb_camera_actor.attributes["enable_postprocess_effects"]) + " [DB Data] : " + str(db_data_list[0][18]))
    if (float(rgb_camera_actor.attributes["gamma"]) != db_data_list[0][19]):
        print("[RGB PARAMETER MISMATCH] Gamma -> [World Data] " + str(rgb_camera_actor.attributes["gamma"]) + " [DB Data] : " + str(db_data_list[0][19]))
    if (float(rgb_camera_actor.attributes["motion_blur_intensity"]) != db_data_list[0][20]):
        print("[RGB PARAMETER MISMATCH] Motion blur intensity -> [World Data] " + str(rgb_camera_actor.attributes["motion_blur_intensity"]) + " [DB Data] : " + str(db_data_list[0][20]))
    if (float(rgb_camera_actor.attributes["motion_blur_max_distortion"]) != db_data_list[0][21]):
        print("[RGB PARAMETER MISMATCH] Motion blur max distortion -> [World Data] " + str(rgb_camera_actor.attributes["motion_blur_max_distortion"]) + " [DB Data] : " + str(db_data_list[0][21]))
    if (float(rgb_camera_actor.attributes["lens_flare_intensity"]) != db_data_list[0][22]):
        print("[RGB PARAMETER MISMATCH] Lens flare intensity -> [World Data] " + str(rgb_camera_actor.attributes["lens_flare_intensity"]) + " [DB Data] : " + str(db_data_list[0][22]))
    if (float(rgb_camera_actor.attributes["motion_blur_min_object_screen_size"]) != db_data_list[0][23]):
        print("[RGB PARAMETER MISMATCH] Motion blur min object screen size -> [World Data] " + str(rgb_camera_actor.attributes["motion_blur_min_object_screen_size"]) + " [DB Data] : " + str(db_data_list[0][23]))
    if (float(rgb_camera_actor.attributes["exposure_min_bright"]) != db_data_list[0][24]):
        print("[RGB PARAMETER MISMATCH] Exposure min bright -> [World Data] " + str(rgb_camera_actor.attributes["exposure_min_bright"]) + " [DB Data] : " + str(db_data_list[0][24]))
    if (float(rgb_camera_actor.attributes["exposure_speed_up"]) != db_data_list[0][25]):
        print("[RGB PARAMETER MISMATCH] Exposure speed up -> [World Data] " + str(rgb_camera_actor.attributes["exposure_speed_up"]) + " [DB Data] : " + str(db_data_list[0][25]))
    if (float(rgb_camera_actor.attributes["exposure_speed_down"]) != db_data_list[0][26]):
        print("[RGB PARAMETER MISMATCH] Exposure speed down -> [World Data] " + str(rgb_camera_actor.attributes["exposure_speed_down"]) + " [DB Data] : " + str(db_data_list[0][26]))
    if (float(rgb_camera_actor.attributes["tint"]) != db_data_list[0][27]):
        print("[RGB PARAMETER MISMATCH] Tint -> [World Data] " + str(rgb_camera_actor.attributes["tint"]) + " [DB Data] : " + str(db_data_list[0][27]))
    if (float(rgb_camera_actor.attributes["calibration_constant"]) != db_data_list[0][28]):
        print("[RGB PARAMETER MISMATCH] Calibration constant -> [World Data] " + str(rgb_camera_actor.attributes["calibration_constant"]) + " [DB Data] : " + str(db_data_list[0][28]))
    if (float(rgb_camera_actor.attributes["focal_distance"]) != db_data_list[0][29]):
        print("[RGB PARAMETER MISMATCH] Focal distance -> [World Data] " + str(rgb_camera_actor.attributes["focal_distance"]) + " [DB Data] : " + str(db_data_list[0][29]))
    if (float(rgb_camera_actor.attributes["min_fstop"]) != db_data_list[0][30]):
        print("[RGB PARAMETER MISMATCH] Min fstop -> [World Data] " + str(rgb_camera_actor.attributes["min_fstop"]) + " [DB Data] : " + str(db_data_list[0][30]))
    if (int(rgb_camera_actor.attributes["blade_count"]) != db_data_list[0][31]):
        print("[RGB PARAMETER MISMATCH] Blade count -> [World Data] " + str(rgb_camera_actor.attributes["blade_count"]) + " [DB Data] : " + str(db_data_list[0][31]))
    if (float(rgb_camera_actor.attributes["blur_amount"]) != db_data_list[0][32]):
        print("[RGB PARAMETER MISMATCH] Blur amount -> [World Data] " + str(rgb_camera_actor.attributes["blur_amount"]) + " [DB Data] : " + str(db_data_list[0][32]))
    if (float(rgb_camera_actor.attributes["blur_radius"]) != db_data_list[0][33]):
        print("[RGB PARAMETER MISMATCH] Blur radius -> [World Data] " + str(rgb_camera_actor.attributes["blur_radius"]) + " [DB Data] : " + str(db_data_list[0][33]))
    if (float(rgb_camera_actor.attributes["slope"]) != db_data_list[0][34]):
        print("[RGB PARAMETER MISMATCH] Slope -> [World Data] " + str(rgb_camera_actor.attributes["slope"]) + " [DB Data] : " + str(db_data_list[0][34]))
    if (float(rgb_camera_actor.attributes["toe"]) != db_data_list[0][35]):
        print("[RGB PARAMETER MISMATCH] Toe -> [World Data] " + str(rgb_camera_actor.attributes["toe"]) + " [DB Data] : " + str(db_data_list[0][35]))
    if (float(rgb_camera_actor.attributes["white_clip"]) != db_data_list[0][36]):
        print("[RGB PARAMETER MISMATCH] White clip -> [World Data] " + str(rgb_camera_actor.attributes["white_clip"]) + " [DB Data] : " + str(db_data_list[0][36]))
    if (float(rgb_camera_actor.attributes["shoulder"]) != db_data_list[0][37]):
        print("[RGB PARAMETER MISMATCH] Shoulder -> [World Data] " + str(rgb_camera_actor.attributes["shoulder"]) + " [DB Data] : " + str(db_data_list[0][37]))
    if (float(rgb_camera_actor.attributes["black_clip"]) != db_data_list[0][38]):
        print("[RGB PARAMETER MISMATCH] Black clip -> [World Data] " + str(rgb_camera_actor.attributes["black_clip"]) + " [DB Data] : " + str(db_data_list[0][38]))
    if (float(rgb_camera_actor.attributes["temp"]) != db_data_list[0][39]):
        print("[RGB PARAMETER MISMATCH] Temp -> [World Data] " + str(rgb_camera_actor.attributes["temp"]) + " [DB Data] : " + str(db_data_list[0][39]))
    if (float(rgb_camera_actor.attributes["chromatic_aberration_intensity"]) != db_data_list[0][40]):
        print("[RGB PARAMETER MISMATCH] Chromatic aberration intensity -> [World Data] " + str(rgb_camera_actor.attributes["chromatic_aberration_intensity"]) + " [DB Data] : " + str(db_data_list[0][40]))


    print("\n\nFinished comparison. Hope it all went good! :)")
    connection.close()

if __name__ == "__main__":
    try: 
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')