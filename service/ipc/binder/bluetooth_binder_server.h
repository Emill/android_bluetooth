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

#include <string>
#include <vector>

#include <base/macros.h>

#include "service/adapter.h"
#include "service/ipc/binder/IBluetooth.h"
#include "service/ipc/binder/IBluetoothCallback.h"
#include "service/ipc/binder/IBluetoothGattServer.h"
#include "service/ipc/binder/IBluetoothLowEnergy.h"
#include "service/ipc/binder/remote_callback_list.h"
#include "service/uuid.h"

namespace ipc {
namespace binder {

// Implements the server side of the IBluetooth Binder interface.
class BluetoothBinderServer : public BnBluetooth,
                              public bluetooth::Adapter::Observer {
 public:
  explicit BluetoothBinderServer(bluetooth::Adapter* adapter);
  ~BluetoothBinderServer() override;

  // IBluetooth overrides:
  bool IsEnabled() override;
  int GetState() override;
  bool Enable() override;
  bool EnableNoAutoConnect() override;
  bool Disable() override;

  std::string GetAddress() override;
  std::vector<bluetooth::UUID> GetUUIDs() override;
  bool SetName(const std::string& name) override;
  std::string GetName() override;

  void RegisterCallback(
      const android::sp<IBluetoothCallback>& callback) override;
  void UnregisterCallback(
      const android::sp<IBluetoothCallback>& callback) override;

  bool IsMultiAdvertisementSupported() override;
  android::sp<IBluetoothLowEnergy> GetLowEnergyInterface() override;
  android::sp<IBluetoothGattServer> GetGattServerInterface() override;

  // bluetooth::Adapter::Observer overrides:
  void OnAdapterStateChanged(bluetooth::Adapter* adapter,
                             bluetooth::AdapterState prev_state,
                             bluetooth::AdapterState new_state) override;

 private:
  bluetooth::Adapter* adapter_;  // weak
  RemoteCallbackList<IBluetoothCallback> callbacks_;

  // The IBluetoothLowEnergy interface handle. This is lazily initialized on the
  // first call to GetLowEnergyInterface().
  android::sp<IBluetoothLowEnergy> low_energy_interface_;

  // The IBluetoothGattServer interface handle. This is lazily initialized on
  // the first call to GetLowEnergyInterface().
  android::sp<IBluetoothGattServer> gatt_server_interface_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothBinderServer);
};

}  // namespace binder
}  // namespace ipc
