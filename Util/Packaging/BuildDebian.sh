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

CARLA_VERSION=$1
DEBIAN_REVISION=$2

DEB_NAME=carla-simulator-${CARLA_VERSION}-${DEBIAN_REVISION}

CARLA_RELEASE_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_${CARLA_VERSION}.tar.gz
ADDITIONAL_MAPS_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz

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
  echo "Downloading CARLA package from ${CARLA_RELEASE_URL}"
  curl "${CARLA_RELEASE_URL}" | tar xz

  echo "Downloading Additional Maps from ${ADDITIONAL_MAPS_URL}"
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

