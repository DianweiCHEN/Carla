# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os

import carla
import pygame
import zipfile

import numpy as np

from carla import ColorConverter as cc
from . import SyncSmokeTest

try:
    import queue
except ImportError:
    import Queue as queue

class Camera(object):
    def __init__(self, sensor, size, record, output_path):
        self.sensor = sensor
        self.size = size
        self.recording = record
        self.image_ready = True
        self.output_path = output_path
        self.image_queue = queue.Queue()
        self.sum_all_white = self.size["height"] * self.size["width"] * 255 * 3

        self.sensor.listen(lambda image: self.parse_image(image) )

    # To avoid float errors to have always the same angle in the output
    def get_closest_angle(self, angle):
        tolerance = 5.0
        angle = angle if (angle >= 0.0) else (360.0 + angle)
        if (360.0 - tolerance) < angle or angle < tolerance:
            return "0"
        if (90.0 - tolerance) < angle and angle < (90.0 + tolerance):
            return "90"
        if (180.0 - tolerance) < angle and angle < (180.0 + tolerance):
            return "180"
        if (270.0 - tolerance) < angle and angle < (270.0 + tolerance):
            return "270"
        return "Invalid"

    def get_transform_as_string(self, transform):
        loc = transform.location
        yaw = self.get_closest_angle(transform.rotation.yaw)
        output = "{:.2f}_{:.2f}_{:.2f}_{}".format(loc.x, loc.y, loc.z, yaw)
        return output

    def parse_image(self, image):
        if self.image_ready:
            return

        str_transform = self.get_transform_as_string(self.sensor.get_transform())
        image_name = '%s/%s.png' % (self.output_path, str_transform)

        if self.recording:
            image.save_to_disk(image_name)
        else:
            image.convert(cc.Raw)
            self.image_queue.put([image_name, image])

        self.image_ready = True

    def destroy(self):
        self.sensor.destroy()

    def image_diff(self, tolerance):
        if self.image_queue.empty():
            return False

        surface = self.image_queue.get_nowait()
        # Open saved sample
        img_path = surface[0]
        sample = pygame.image.load(img_path)

        image = surface[1]
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (self.size["height"], self.size["width"], 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        array = array.swapaxes(0, 1)
        surface = pygame.surfarray.make_surface(array)

        img1_arr = pygame.PixelArray(sample)
        img2_arr = pygame.PixelArray(surface)
        # This returns a grayscale image
        # white means same images are equal
        diff_arr = img2_arr.compare(img1_arr, distance=0.05)

        # Check failure
        array = np.array(pygame.surfarray.array3d(diff_arr.surface))
        print(array)
        print("Sum:", array.sum())
        total = self.sum_all_white - array.sum()

        if total < (self.sum_all_white * tolerance) or True:
            print("total =",total,"->", self.sum_all_white - total)
            path = img_path.split("/")
            path[0] = "errors"
            full_path = '/'.join(path)
            print(full_path)
            image.save_to_disk(full_path)
            path[2] = "diff_" + path[2]
            full_path = '/'.join(path)
            pygame.image.save(diff_arr.surface, full_path)

        img1_arr.close()
        img2_arr.close()
        diff_arr.close()

        return self.image_queue.empty()


class TestSampleMap(SyncSmokeTest):
    def test_sample_map(self):
        print("TestSampleMap")

        self.record = False
        self.root_output = "map_samples/"

        error = self.unzip_samples()
        # TODO: cancel everything if this fails

        # get all available maps
        maps = self.client.get_available_maps()
        maps = ["Town03"]
        print(maps)
        for m in maps:
            print("Loading", m)
            # load the map
            self.client.load_world(m)
            self.world = self.client.get_world()

            # Disable weather features
            weather = self.world.get_weather()
            weather.cloudiness = 0.0
            weather.precipitation = 0.0
            weather.wind_intensity = 0.0
            weather.fog_density = 0.0
            weather.fog_distance = 0.0
            self.world.set_weather(weather)

            # get all spawn points
            spawn_points = self.world.get_map().get_spawn_points()
            spawn_points = [spawn_points[i] for i in range(1)]
            print("Num spawn_points:", len(spawn_points))

            # Spawn cameras
            self.cameras = []
            image_size = {"width":960, "height":540}
            self.output = self.root_output + m

            bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
            bp.set_attribute('image_size_x', str(image_size["width"]))
            bp.set_attribute('image_size_y', str(image_size["height"]))
            bp.set_attribute('fov', "90.0")

            self.cameras.append(Camera(
                self.world.spawn_actor(bp, carla.Transform(rotation = carla.Rotation(yaw = 0.0))),
                image_size,
                self.record,
                self.output
            ))
            self.cameras.append(Camera(
                self.world.spawn_actor(bp, carla.Transform(rotation = carla.Rotation(yaw = 90.0))),
                image_size,
                self.record,
                self.output
            ))
            self.cameras.append(Camera(
                self.world.spawn_actor(bp, carla.Transform(rotation = carla.Rotation(yaw = 180.0))),
                image_size,
                self.record,
                self.output
            ))
            self.cameras.append(Camera(
                self.world.spawn_actor(bp, carla.Transform(rotation = carla.Rotation(yaw = 270.0))),
                image_size,
                self.record,
                self.output
            ))

            # Apply sync mode
            settings = self.world.get_settings()
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=False,
                fixed_delta_seconds=0.05)
            self.world.apply_settings(settings)

            for spawn_point in spawn_points:
                spawn_point.location.z += 1.5
                # Reallocate cameras
                for camera in self.cameras:
                    camera.sensor.set_location(spawn_point.location)

                # Discard some images
                self.tick()
                self.tick()
                self.tick()

                for camera in self.cameras:
                    camera.image_ready = False # reset sync flag

                while not self.ready_for_tick():
                    continue

                self.tick()

        # Wait until is job pending
        while(self.calculate_camera_diff()):
            continue

        # Apply async mode
        settings = self.world.get_settings()
        settings.synchronous_mode = False
        self.world.apply_settings(settings)
        self.tick()

        for camera in self.cameras:
            camera.destroy()

        if self.record:
            self.zip_output()

    def tick(self):
        self.world.tick()
        self.calculate_camera_diff()

    def calculate_camera_diff(self):
        error_tolerance = 0.05
        job_pending = False
        for camera in self.cameras:
            job_pending |= camera.image_diff(error_tolerance)
        return job_pending

    def ready_for_tick(self):
        for camera in self.cameras:
            if not camera.image_ready:
                return False
        return True

    """
    # Some spawn_points are so close that we can ignore them
    def filter_spawn_points(self, spawn_points):
        result = []
        num_points = len(spawn_points)
        threshold = 2.5
        for i in range(num_points):
            loc1 = spawn_points[i].location
            for j in range(i+1, num_points):
                loc2 = spawn_points[i].location
                dist = (loc2 - loc1).distance()
                if dist < threshold:
    """

    def zip_output(self):
        zipf = zipfile.ZipFile('maps_samples.zip', 'w', zipfile.ZIP_DEFLATED)
        for root, dirs, files in os.walk(self.root_output):
            for file in files:
                print(file)
                zipf.write(os.path.join(root, file))
        zipf.close()

    def unzip_samples(self):
        if os.path.isfile('maps_samples.zip'):
            with zipfile.ZipFile('maps_samples.zip', 'r') as zip_ref:
                zip_ref.extractall("./")
        elif not self.record:
            print("[ERROR]: Map samples not founded!")
            return False
        return True


