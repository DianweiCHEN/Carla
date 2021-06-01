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

    #Compare weather parameters with simulation
    print("Comparing WEATHER PARAMETERS data with simulation\n\n")
    db_data = c.execute('''SELECT * FROM WeatherParameters''')
    db_data_list = []
    for value in db_data:
        db_data_list.append(value)
    if (weather.cloudiness != db_data_list[0][0]):
        print("[WEATHER MISMATCH] Cloudiness -> [World Data] " + str(db_data_list[0][0]) + " [DB Data] : " + str(db_data_list[0][0]))
    if (weather.precipitation != db_data_list[0][1]):
        print("[WEATHER MISMATCH] Precipitation -> [World Data] " + str(db_data_list[0][1]) + " [DB Data] : " + str(db_data_list[0][1]))
    if (weather.precipitation_deposits != db_data_list[0][2]):
        print("[WEATHER MISMATCH] Precipitation Deposits -> [World Data] " + str(db_data_list[0][2]) + " [DB Data] : " + str(db_data_list[0][2]))
    if (weather.wind_intensity != db_data_list[0][3]):
        print("[WEATHER MISMATCH] Wind Intensity -> [World Data] " + str(db_data_list[0][3]) + " [DB Data] : " + str(db_data_list[0][3]))
    if (weather.sun_azimuth_angle != db_data_list[0][4]):
        print("[WEATHER MISMATCH] Sun Azimuth Angle -> [World Data] " + str(db_data_list[0][4]) + " [DB Data] : " + str(db_data_list[0][4]))
    if (weather.sun_altitude_angle != db_data_list[0][5]):
        print("[WEATHER MISMATCH] Sun Altitude Angle -> [World Data] " + str(db_data_list[0][5]) + " [DB Data] : " + str(db_data_list[0][5]))
    if (weather.fog_density != db_data_list[0][6]):
        print("[WEATHER MISMATCH] Fog Density -> [World Data] " + str(db_data_list[0][6]) + " [DB Data] : " + str(db_data_list[0][6]))
    if (weather.fog_distance != db_data_list[0][7]):
        print("[WEATHER MISMATCH] Fog Distance -> [World Data] " + str(db_data_list[0][7]) + " [DB Data] : " + str(db_data_list[0][7]))
    if (weather.wetness != db_data_list[0][8]):
        print("[WEATHER MISMATCH] Wetness -> [World Data] " + str(db_data_list[0][8]) + " [DB Data] : " + str(db_data_list[0][8]))
    if (weather.fog_falloff != db_data_list[0][9]):
        print("[WEATHER MISMATCH] Fog Falloff -> [World Data] " + str(db_data_list[0][9]) + " [DB Data] : " + str(db_data_list[0][9]))

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

    print("Finished comparison. Hope it all went good! :)")
    connection.close()

if __name__ == "__main__":
    try: 
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')