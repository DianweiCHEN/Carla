#!/bin/bash

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==================================================================================================
# -- Install CARLA ---------------------------------------------------------------------------------
# ==================================================================================================
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1AF1527DE64CB8D9
sudo add-apt-repository "deb [arch=amd64] http://dist.carla.org/carla $(lsb_release -sc) main"
sudo apt-get update
sudo apt-get install carla-simulator=0.9.11 -y

# ==================================================================================================
# -- Install RLlib ---------------------------------------------------------------------------------
# ==================================================================================================
source activate tensorflow2_p36
pip3 install pygame paramiko scp ray[rllib]

# ==================================================================================================
# -- Env variables ---------------------------------------------------------------------------------
# ==================================================================================================
echo "export CARLA_ROOT=/opt/carla-simulator" >> ~/.bashrc
echo "source activate tensorflow2_p36" >> ~/.bashrc
echo 'export PYTHONPATH=""' >> ~/.bashrc
echo 'export PYTHONPATH=$PYTHONPATH:"${CARLA_ROOT}/PythonAPI/carla/dist/$(ls ${CARLA_ROOT}/PythonAPI/carla/dist | grep py3.)"' >> ~/.bashrc
echo 'export PYTHONPATH=$PYTHONPATH:"${CARLA_ROOT}/PythonAPI/carla"' >> ~/.bashrc
