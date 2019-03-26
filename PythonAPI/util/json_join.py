#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generate map from FBX"""

import os
import argparse

def parse_arguments():
    """Arguments parser."""
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-f', '--folder',
        metavar='F',
        type=str,
        help='Path where the json files to join are located')
    argparser.add_argument(
        '-n', '--name',
        metavar='N',
        type=str,
        help='Name of the root json component')
    return argparser.parse_args()

def main():
    """Main entrance point of the script."""
    try:
        args = parse_arguments()
        dirname = os.path.abspath(args.folder)
        print os.path.join(dirname, "outputfile.json")
        cat_json(os.path.join(dirname, "outputfile.json"), dirname, args.name)

    finally:
        print 'done.'

def cat_json(outfile, path, header):
    """Method to concat jsons"""
    print path
    for r, d, infiles in os.walk(path):
        print r
        print d
        with file(outfile, "w") as outfile:
            first = True
            outfile.write('{\"%s\":' % header)
            outfile.write('[')
            for infile_name in sorted(infiles):
                print "Processing %s" % infile_name
                with file(os.path.join(path, infile_name)) as infile:
                    if not first:
                        outfile.write(',')
                    else:
                        first = False
                    outfile.write(infile.read())
            outfile.write(']')
            outfile.write('}')

if __name__ == '__main__':
    main()
