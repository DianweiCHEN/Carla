#!/usr/bin/env python3

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Script for training.
"""
import argparse
import logging
import os
import sys

from aws import runner


def real_main():
    print("Hello from real main")
    return


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('--aws', action='store_true', help='enable ec2 instance')
    argparser.add_argument("--instance-id",
                           default="",
                           help="EC2 instance to connect")
    argparser.add_argument("--stop", action="store_true", help="enable debug messages")
    argparser.add_argument("--debug", action="store_true", help="enable debug messages")
    arguments = argparser.parse_args()

    if arguments.debug:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    if arguments.aws:
        args = sys.argv[1:]
        args.remove("--aws")
        aws_runner = runner.AWSRunner(arguments.instance_id)
        aws_runner.run(os.path.realpath(__file__), " ".join(args))
        if arguments.stop:
            aws_runner.stop()

    else:
        real_main()

    sys.exit()
