#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exports maps from UE4 to .tar.gz"""

import os
import json
import subprocess
import shutil
import argparse
import tarfile
import glob

if os.name == 'nt':
    sys_name = 'Win64'
elif os.name == 'posix':
    sys_name = 'Linux'

def parse_arguments():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-d', '--dir',
        metavar='D',
        type=str,
        default='ExportedMaps',
        help='Output Directory in which the map will be cooked and packaged')
    argparser.add_argument(
        '-m', '--map',
        metavar='M',
        type=str,
        help='Map path in UE4 style \'/Game/Carla/Maps/ExampleMap\'')
    argparser.add_argument(
        '--file',
        metavar='F',
        type=str,
        default='CookedExportedMaps',
        help='File name for the .tar.gz final file')
    argparser.add_argument(
        '--targetplatform',
        metavar='T',
        type=str,
        default='LinuxNoEditor',
        help='Target for the cook command. WindowsNoEditor or LinuxNoEditor'
    )
    return argparser.parse_args()

def main():
    args = parse_arguments()
    dirname = os.getcwd()

    output_dir = os.path.join(dirname, "..", "..", "%s" % args.dir)
    filename = os.path.join(output_dir, "%s.tar.gz" % args.file)
    build_dir = output_dir

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    map_list = args.map
    if not map_list:
        map_list =  []
        map_place = os.path.join(dirname, "..", "..", "Unreal", "CarlaUE4", "Content", "Carla", "ExportedMaps")
        for filename in os.listdir(map_place):
            if filename.endswith('.umap'):
                map_list.append(filename)
    project_path = os.path.join(dirname, "..", "..", "Unreal", "CarlaUE4")

    ue4_path = os.environ['UE4_ROOT']
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    cooked_content = os.path.join(build_dir, "Cooked", "CarlaUE4")
    for single_map in map_list:
        full_command = "%s \"%s/CarlaUE4.uproject\" -run=cook -map=%s -cooksinglepackage -targetplatform=\"%s\" -OutputDir=\"%s/Cooked\"" % (editor_path, project_path, single_map, args.targetplatform, output_dir)
        subprocess.check_call([full_command], shell=True)
        shutil.rmtree(os.path.join(cooked_content, "Metadata"))
        shutil.rmtree(os.path.join(cooked_content, "Plugins"))
        os.remove(os.path.join(cooked_content, "AssetRegistry.bin"))
        tar = tarfile.open(os.path.join(output_dir, os.path.splitext(single_map)[0] + ".tar.gz"), "w:gz")
        for file_name in glob.glob(os.path.join("%s" % build_dir, "Cooked", "*")):
            print("  Adding %s... This may take a while" % file_name)
            tar.add(file_name, os.path.basename(file_name))
        tar.close()
        shutil.rmtree(os.path.join(build_dir, "Cooked"))
    print(map_list)

if __name__ == '__main__':
    try:
        main()
    finally:
        print("Finished")