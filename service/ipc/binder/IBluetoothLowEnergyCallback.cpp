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

#include "service/ipc/binder/IBluetoothLowEnergyCallback.h"

#include <base/logging.h>
#include <binder/Parcel.h>

#include "service/ipc/binder/parcel_helpers.h"

using android::IBinder;
using android::Parcel;
using android::sp;
using android::status_t;

using bluetooth::AdvertiseData;
using bluetooth::AdvertiseSettings;

namespace ipc {
namespace binder {

// static
const char IBluetoothLowEnergyCallback::kServiceName[] =
    "bluetooth-low-energy-callback-service";

// BnBluetoothLowEnergyCallback (server) implementation
// ========================================================

status_t BnBluetoothLowEnergyCallback::onTransact(
    uint32_t code,
    const Parcel& data,
    Parcel* reply,
    uint32_t flags) {
  VLOG(2) << "IBluetoothLowEnergyCallback: " << code;
  if (!data.checkInterface(this))
    return android::PERMISSION_DENIED;

  switch (code) {
  case ON_CLIENT_REGISTERED_TRANSACTION: {
    int status = data.readInt32();
    int client_if = data.readInt32();
    OnClientRegistered(status, client_if);
    return android::NO_ERROR;
  }
  case ON_MULTI_ADVERTISE_CALLBACK_TRANSACTION: {
    int status = data.readInt32();
    bool is_start = data.readInt32();
    std::unique_ptr<AdvertiseSettings> settings =
        CreateAdvertiseSettingsFromParcel(data);
    OnMultiAdvertiseCallback(status, is_start, *settings);
    return android::NO_ERROR;
  }
  default:
    return BBinder::onTransact(code, data, reply, flags);
  }
}

// BpBluetoothLowEnergyCallback (client) implementation
// ========================================================

BpBluetoothLowEnergyCallback::BpBluetoothLowEnergyCallback(
    const sp<IBinder>& impl)
    : BpInterface<IBluetoothLowEnergyCallback>(impl) {
}

void BpBluetoothLowEnergyCallback::OnClientRegistered(
    int status, int client_if) {
  Parcel data, reply;

  data.writeInterfaceToken(
      IBluetoothLowEnergyCallback::getInterfaceDescriptor());
  data.writeInt32(status);
  data.writeInt32(client_if);

  remote()->transact(
      IBluetoothLowEnergyCallback::ON_CLIENT_REGISTERED_TRANSACTION,
      data, &reply,
      IBinder::FLAG_ONEWAY);
}

void BpBluetoothLowEnergyCallback::OnMultiAdvertiseCallback(
    int status, bool is_start,
    const AdvertiseSettings& settings) {
  Parcel data, reply;

  data.writeInterfaceToken(
      IBluetoothLowEnergyCallback::getInterfaceDescriptor());
  data.writeInt32(status);
  data.writeInt32(is_start);
  WriteAdvertiseSettingsToParcel(settings, &data);

  remote()->transact(
      IBluetoothLowEnergyCallback::ON_MULTI_ADVERTISE_CALLBACK_TRANSACTION,
      data, &reply,
      IBinder::FLAG_ONEWAY);
}

IMPLEMENT_META_INTERFACE(BluetoothLowEnergyCallback,
                         IBluetoothLowEnergyCallback::kServiceName);

}  // namespace binder
}  // namespace ipc
