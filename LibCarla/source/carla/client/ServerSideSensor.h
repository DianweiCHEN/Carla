// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/Simulator.h"
#include "carla/client/Sensor.h"

namespace carla {
namespace client {

  class ServerSideSensor final : public Sensor {
  public:

    using Sensor::Sensor;

    ~ServerSideSensor();

    /// Register a @a callback to be executed each time a new measurement is
    /// received.
    ///
    /// @warning Calling this function on a sensor that is already listening
    /// steals the data stream from the previously set callback. Note that
    /// several instances of Sensor (even in different processes) may point to
    /// the same sensor in the simulator.
    void Listen(CallbackFunctionType callback) override;
    void ListenPasive() override;

    /// Stop listening for new measurements.
    void Stop() override;

    /// Return whether this Sensor instance is currently listening to the
    /// associated sensor in the simulator.
    bool IsListening() const override {
      return _is_listening;
    }

    bool IsPasive() const override {
      return _is_pasive;
    }

    bool HasDataReady() const override {
      if (!_last_data) return false;
      
      SharedPtr<sensor::SensorData> data = _last_data;
      if (data && data->GetFrame() == GetEpisode().Lock()->GetWorldSnapshot().GetFrame())
        return true;
      return false;
    }

    /// @copydoc Actor::Destroy()
    ///
    /// Additionally stop listening.
    bool Destroy() override;

  private:

    bool _is_pasive = false;
    bool _is_listening = false;
  };

} // namespace client
} // namespace carla
