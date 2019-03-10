


def get_scene_layout(world):

    """
    Function to extract the full scene layout to be used as a full scene description to be
    given to the user
    :param world: the world object from CARLA
    :return: a dictionary describing the scene.
    """

    ######## PARSE HERE THE MAP WITH THE NECESSARY FUNCTIONS. REUSE NON RENDERING MODE


    return {


        "1": {
            "road_id": 0,
            "lane_id": 0,
            "lat": 0.2313,
            "lon": 0.233,
            "z": 30,
            "left_margin_gps_and_z": {'lat': 0.3, 'lon': 0.1, 'z': 23},
            "right_margin_gps_and_z": {'lat': 0.1, 'lon': 0.01, 'z': 23},
            "next_waypoints_ids": ["2"],
            "left_lane_waypoint_id": -1,
            "right_lane_waypoint_id": -1

        },
        "2": {
            "road_id": 0,
            "lane_id": 0,
            "lat": 0.2313,
            "lon": 0.233,
            "z": 30,
            "left_margin_gps_and_z": {'lat': 0.3, 'lon': 0.1, 'z': 23},
            "right_margin_gps_and_z": {'lat': 0.1, 'lon': 0.01, 'z': 23},
            "next_waypoints_ids": ["4", "5"],
            "left_lane_waypoint_id": -1,
            "right_lane_waypoint_id": -1

        }

    }


def get_dynamic_objects(world):

    def get_stop_signals(stopsignals):

        return [ {
                    "id": 99,
                    "position": [0,0,0], # x,y,z
                    "trigger_volume": [[],[]] # Set of points

                }

        ]

    def get_traffic_lights(traffic_lights):
        return {
            "id": 99,
            "state": 0,  # Red yellow grean
            "position": [0, 0, 0],  # x,y,z
            "trigger_volume": [[], []]  # Set of points

        }

    def get_vehicles(vehicles):


         return [{
             "id": 99,
              "position": [0,0,0], #x,y,z
              "orientation": [0,0,0], # roll pitch yaw
              "bounding_box": [[]] # set of points
            }
         ]

    def get_walkers(walkers):

        return {
            "id": 99,
            "position": [0, 0, 0],  # x,y,z
            "orientation": [0, 0, 0],  # roll pitch yaw
            "bounding_box": [[]]  # set of points
        }

    def get_speed_limits(speed_limits):



        return {
            {
                "id": 99,
                "position": [0, 0, 0],  # x,y,z
                "speed": 90  # Set of points

            }


        }

    return {
        'vehicles': get_vehicles(None),  # add the appropriate values baserd on world
        'walkers': get_walkers(None),
        'traffic_lights': get_traffic_lights(None),
        'stop_signs': get_stop_signals(None),
        'speed_limits': get_speed_limits(None)

    }
