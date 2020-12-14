# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import time

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
        self.offset = 20
        self.sum_subarray_white = self.offset * self.offset * 255 * 3

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

    def image_diff(self):
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
        diff_arr = img2_arr.compare(img1_arr, distance=0.1) #, weights=(1.0, 1.0, 1.0))

        # Check failure (10 x 10 cells)
        array = np.array(pygame.surfarray.array3d(diff_arr.surface))
        total_error = 0
        for i in range(0, self.size["width"], self.offset):
            for j in range(0, self.size["height"], self.offset):
                sub_array = array[i:i+self.offset, j:j+self.offset, :]
                subtotal = 1.0 - (sub_array.sum() / self.sum_subarray_white)
                # print("{} / {} = {:.2f} -> {:.2f}".format(sub_array.sum(), self.sum_subarray_white, (sub_array.sum() / self.sum_subarray_white), subtotal))
                total_error += subtotal if subtotal >= 0.5 else 0.0
        if total_error > 0.0:
            w, h = surface.get_size()
            for x in range(w):
                for y in range(h):
                    error = diff_arr.surface.get_at((x, y))[0]
                    r, g, b, a = surface.get_at((x, y))
                    final_r = r if error > 0 else 255
                    diff_arr.surface.set_at((x, y), pygame.Color(final_r, g, b, a))
            path = img_path.split("/")
            # print("[Error] ({:.2f}) {}".format(total_error, path[2]))
            path[0] = "errors"
            full_path = '/'.join(path)
            image.save_to_disk(full_path)
            name_split = path[2].split(".")
            name_split[len(name_split) - 1] = name_split[len(name_split) - 2] + "_diff"
            path[2] = '.'.join(name_split)
            full_path = '/'.join(path)
            pygame.image.save(diff_arr.surface.copy(), full_path)

        img1_arr.close()
        img2_arr.close()
        diff_arr.close()

        return self.image_queue.empty()


class TestSampleMap(SyncSmokeTest):
    def test_sample_map(self):
        print("TestSampleMap")
        pygame.init()

        self.record = False

        valid_zip = self.unzip_samples()

        if not self.record:
            self.assertTrue(valid_zip, "Zip samples file is not found or invalid!")

        # get all available maps
        maps = self.client.get_available_maps()
        maps = ["Town01", "Town01_Opt",
                "Town02", "Town02_Opt",
                "Town03", "Town03_Opt",
                "Town04", "Town04_Opt",
                "Town05", "Town05_Opt",
                "Town06", "Town06_Opt",
                "Town07", "Town07_Opt",
                "Town08", "Town08_Opt",
                "Town09", "Town09_Opt",
                "Town10HD", "Town10HD_Opt"]
        for m in maps:
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

            ref_points = self.get_reference_points()

            # Spawn cameras
            self.cameras = []
            image_size = {"width":int(1280/2), "height":int(720/2)}
            self.output = "map_samples/" + m

            bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
            bp.set_attribute('image_size_x', str(image_size["width"]))
            bp.set_attribute('image_size_y', str(image_size["height"]))
            bp.set_attribute('exposure_mode', "manual")
            bp.set_attribute('fov', "90.0")

            camera_transform = [
                carla.Transform(rotation = carla.Rotation(yaw =   0.0)),
                carla.Transform(rotation = carla.Rotation(yaw =  90.0)),
                carla.Transform(rotation = carla.Rotation(yaw = 180.0)),
                carla.Transform(rotation = carla.Rotation(yaw = 270.0))
            ]

            for i in range(4):
                self.cameras.append(Camera(
                    self.world.spawn_actor(bp, camera_transform[i]),
                    image_size,
                    self.record,
                    self.output
                ))

            # Apply sync mode
            self.world.apply_settings(carla.WorldSettings(
                synchronous_mode=True,
                no_rendering_mode=False,
                fixed_delta_seconds=0.05))

            current_sp = 0
            for ref_point in ref_points:
                current_sp += 1
                print("{} {:0.2f} % ({}/{})".format(m, 100.0 * current_sp/len(ref_points), current_sp, len(ref_points)), end="\r")

                ref_point.z += 1.5
                # Reallocate cameras
                for camera in self.cameras:
                    camera.sensor.set_location(ref_point)

                # Discard some images to avoid some artifacts
                for i in range(10):
                    self.tick()

                for camera in self.cameras:
                    camera.image_ready = False # reset sync flag

                while not self.ready_for_tick():
                    continue

                self.tick()
            if len(ref_points) > 0:
                print("{} {:0.2f} % ({}/{})".format(m, 100.0 * current_sp/len(ref_points), current_sp, len(ref_points)))
            else:
                print("[Error] {} has no reference points".format(m))

            # Wait until is job pending
            while(self.calculate_camera_diff()):
                continue

            for camera in self.cameras:
                camera.destroy()

        # Wait until is job pending
        while(self.calculate_camera_diff()):
            continue

        # Apply async mode
        self.world.apply_settings(carla.WorldSettings(
            synchronous_mode=False,
            no_rendering_mode=False,
            fixed_delta_seconds=0.0))

        if self.record:
            self.zip_output('map_samples.zip', "map_samples/")
        else:
            self.zip_output('errors.zip', "errors/")

        pygame.quit()

    def tick(self):
        self.world.tick()
        self.calculate_camera_diff()

    def get_reference_points(self):
        spawn_points = self.world.get_map().get_spawn_points()
        # print("spawn_points:", len(spawn_points))
        used_sp = set()
        threshold_dist = 5.0
        ref_points = []
        for i in range(len(spawn_points)):
            i_loc = spawn_points[i].location
            merged = False
            # if not used yet
            if not {i}.issubset(used_sp):
                for j in range(i+1, len(spawn_points)):
                    # if not used yet
                    if not {j}.issubset(used_sp):
                        j_loc = spawn_points[j].location
                        dist = i_loc.distance(j_loc)
                        if dist < threshold_dist:
                            ref_point = (i_loc + j_loc) * 0.5
                            ref_points.append(ref_point)
                            used_sp.add(j)
                            merged = True
                if not merged:
                    ref_points.append(i_loc)

        # print("ref_points:",len(ref_points))
        return ref_points

    def calculate_camera_diff(self):
        job_pending = False
        for camera in self.cameras:
            job_pending |= camera.image_diff()
        return job_pending

    def ready_for_tick(self):
        for camera in self.cameras:
            if not camera.image_ready:
                return False
        return True

    def zip_output(self, output_name, path_to_zip):
        if os.path.exists(path_to_zip):
            zipf = zipfile.ZipFile(output_name, 'w', zipfile.ZIP_DEFLATED)
            for root, dirs, files in os.walk(path_to_zip):
                for file in files:
                    zipf.write(os.path.join(root, file))
            zipf.close()

    def unzip_samples(self):
        if os.path.isfile('map_samples.zip'):
            with zipfile.ZipFile('map_samples.zip', 'r') as zip_ref:
                zip_ref.extractall("./")
        elif not self.record:
            print("[ERROR]: Map samples not founded!")
            return False
        return True


