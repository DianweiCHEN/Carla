#!/bin/bash

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# This script builds a debian package for CARLA.
#
# Usage:
#     $ ./CreateDebian.sh <CARLA-VERSION>
#

# ==================================================================================================
# -- Variables -------------------------------------------------------------------------------------
# ==================================================================================================

if [[ -z $1 ]];
then
  echo "$(date) - Missing mandatory arguments: CARLA version. "
  echo "$(date) - Usage: ./CreateDebian.sh [version]. "
  exit 1
fi

#VERSION=$1
#PATCH_VERSION=4
VERSION=0.9.10-Pre_Ubuntu16
PATCH_VERSION=0
#CARLA_VERSION=${VERSION}.${PATCH_VERSION}
CARLA_VERSION=${VERSION}
DEBIAN_REVISION=$((${PATCH_VERSION} + 1))

REF=0

DEB_NAME=carla-simulator-${VERSION}-${DEBIAN_REVISION}

CARLA_RELEASE_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_${CARLA_VERSION}.tar.gz
ADDITIONAL_MAPS_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz

#https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_0.9.10-Pre_Ubuntu18.tar.gz
#https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_0.9.10-Pre_Ubuntu18.tar.gz
#CARLA_RELEASE_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_${CARLA_VERSION}.tar.gz
#ADDITIONAL_MAPS_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz

mkdir -p build/${DEB_NAME} && cd build/${DEB_NAME}
cp -r ../../debian .

# ==================================================================================================
# -- Download --------------------------------------------------------------------------------------
# ==================================================================================================

# Add skip download
FILE=$(pwd)/ImportAssets.sh
if [ -f "$FILE" ]; then
  echo "Package already downloaded!"
else
  curl "${CARLA_RELEASE_URL}" | tar xz

  wget "${ADDITIONAL_MAPS_URL}"
  mv AdditionalMaps_"${CARLA_VERSION}".tar.gz Import/
fi

# Importing new maps.
./ImportAssets.sh

# Removing unnecessary files
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.debug
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.sym

# Updating CarlaUE4.sh script
rm CarlaUE4.sh
cat >> CarlaUE4.sh <<EOF
#!/bin/sh
"/opt/carla-simulator/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping" CarlaUE4 \$@
EOF

# ==================================================================================================
# -- Debian package --------------------------------------------------------------------------------
# ==================================================================================================

# Building debian package.
dpkg-buildpackage -uc -us -b

