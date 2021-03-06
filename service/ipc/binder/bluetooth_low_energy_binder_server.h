//
//  Copyright (C) 2015 Google, Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at:
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#pragma once

#include <memory>

#include <base/macros.h>

#include "service/ipc/binder/IBluetoothLowEnergy.h"
#include "service/ipc/binder/IBluetoothLowEnergyCallback.h"
#include "service/ipc/binder/interface_with_clients_base.h"
#include "service/low_energy_client.h"
#include "service/low_energy_constants.h"

namespace bluetooth {
class Adapter;
}  // namespace bluetooth

namespace ipc {
namespace binder {

// Implements the server side of the IBluetoothLowEnergy interface.
class BluetoothLowEnergyBinderServer : public BnBluetoothLowEnergy,
                                       public InterfaceWithClientsBase {
 public:
  explicit BluetoothLowEnergyBinderServer(bluetooth::Adapter* adapter);
  ~BluetoothLowEnergyBinderServer() override;

  // IBluetoothLowEnergy overrides:
  bool RegisterClient(
      const android::sp<IBluetoothLowEnergyCallback>& callback) override;
  void UnregisterClient(int client_if) override;
  void UnregisterAll() override;
  bool StartMultiAdvertising(
      int client_if,
      const bluetooth::AdvertiseData& advertise_data,
      const bluetooth::AdvertiseData& scan_response,
      const bluetooth::AdvertiseSettings& settings) override;
  bool StopMultiAdvertising(int client_if) override;

 private:
  // Returns a pointer to the IBluetoothLowEnergyCallback instance associated
  // with |client_if|. Returns NULL if such a callback cannot be found.
  android::sp<IBluetoothLowEnergyCallback> GetLECallback(int client_if);

  // Returns a pointer to the LowEnergyClient instance associated with
  // |client_if|. Returns NULL if such a client cannot be found.
  std::shared_ptr<bluetooth::LowEnergyClient> GetLEClient(int client_if);

  // InterfaceWithClientsBase override:
  void OnRegisterClientImpl(
      bluetooth::BLEStatus status,
      android::sp<IInterface> callback,
      bluetooth::BluetoothClientInstance* client) override;

  bluetooth::Adapter* adapter_;  // weak

  DISALLOW_COPY_AND_ASSIGN(BluetoothLowEnergyBinderServer);
};

}  // namespace binder
}  // namespace ipc
