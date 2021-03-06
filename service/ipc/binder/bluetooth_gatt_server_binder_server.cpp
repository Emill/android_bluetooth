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

#include "service/ipc/binder/bluetooth_gatt_server_binder_server.h"

#include <base/logging.h>

#include "service/adapter.h"

namespace ipc {
namespace binder {

namespace {

const int kInvalidClientId = -1;

}  // namespace

BluetoothGattServerBinderServer::BluetoothGattServerBinderServer(
    bluetooth::Adapter* adapter) : adapter_(adapter) {
  CHECK(adapter_);
}

bool BluetoothGattServerBinderServer::RegisterServer(
    const android::sp<IBluetoothGattServerCallback>& callback) {
  VLOG(2) << __func__;
  bluetooth::GattServerFactory* gatt_server_factory =
      adapter_->GetGattServerFactory();

  return RegisterClientBase(callback, gatt_server_factory);
}

void BluetoothGattServerBinderServer::UnregisterServer(int server_if) {
  VLOG(2) << __func__;
  UnregisterClientBase(server_if);
}

void BluetoothGattServerBinderServer::UnregisterAll() {
  VLOG(2) << __func__;
  UnregisterAllBase();
}

bool BluetoothGattServerBinderServer::BeginServiceDeclaration(
    int server_if, bool is_primary, const bluetooth::UUID& uuid,
    std::unique_ptr<bluetooth::GattIdentifier>* out_id) {
  VLOG(2) << __func__;
  CHECK(out_id);
  std::lock_guard<std::mutex> lock(*maps_lock());

  auto gatt_server = GetGattServer(server_if);
  if (!gatt_server) {
    LOG(ERROR) << "Unknown server_if: " << server_if;
    return false;
  }

  auto service_id = gatt_server->BeginServiceDeclaration(uuid, is_primary);
  if (!service_id) {
    LOG(ERROR) << "Failed to begin service declaration - server_if: "
               << server_if << " UUID: " << uuid.ToString();
    return false;
  }

  out_id->swap(service_id);

  return true;
}

bool BluetoothGattServerBinderServer::AddCharacteristic(
    int server_if, const bluetooth::UUID& uuid,
    int properties, int permissions,
    std::unique_ptr<bluetooth::GattIdentifier>* out_id) {
  VLOG(2) << __func__;
  CHECK(out_id);
  std::lock_guard<std::mutex> lock(*maps_lock());

  auto gatt_server = GetGattServer(server_if);
  if (!gatt_server) {
    LOG(ERROR) << "Unknown server_if: " << server_if;
    return false;
  }

  auto char_id = gatt_server->AddCharacteristic(uuid, properties, permissions);
  if (!char_id) {
    LOG(ERROR) << "Failed to add characteristic - server_if: "
               << server_if << " UUID: " << uuid.ToString();
    return false;
  }

  out_id->swap(char_id);

  return true;
}

bool BluetoothGattServerBinderServer::EndServiceDeclaration(int server_if) {
  VLOG(2) << __func__;
  std::lock_guard<std::mutex> lock(*maps_lock());

  auto gatt_server = GetGattServer(server_if);
  if (!gatt_server) {
    LOG(ERROR) << "Unknown server_if: " << server_if;
    return false;
  }

  // Create a weak pointer and pass that to the callback to prevent a potential
  // use after free.
  android::wp<BluetoothGattServerBinderServer> weak_ptr_to_this(this);
  auto callback = [=](
      bluetooth::BLEStatus status, const bluetooth::GattIdentifier& server_id) {
    auto sp_to_this = weak_ptr_to_this.promote();
    if (!sp_to_this.get()) {
      VLOG(2) << "BluetoothLowEnergyBinderServer was deleted";
      return;
    }

    std::lock_guard<std::mutex> lock(*maps_lock());

    auto gatt_cb = GetGattServerCallback(server_if);
    if (!gatt_cb.get()) {
      VLOG(2) << "The callback was deleted";
      return;
    }

    gatt_cb->OnServiceAdded(status, server_id);
  };

  if (!gatt_server->EndServiceDeclaration(callback)) {
    LOG(ERROR) << "Failed to end service declaration";
    return false;
  }

  return true;
}

android::sp<IBluetoothGattServerCallback>
BluetoothGattServerBinderServer::GetGattServerCallback(int server_if) {
  auto cb = GetCallback(server_if);
  return android::sp<IBluetoothGattServerCallback>(
      static_cast<IBluetoothGattServerCallback*>(cb.get()));
}

std::shared_ptr<bluetooth::GattServer>
BluetoothGattServerBinderServer::GetGattServer(int server_if) {
  return std::static_pointer_cast<bluetooth::GattServer>(
      GetClientInstance(server_if));
}

void BluetoothGattServerBinderServer::OnRegisterClientImpl(
    bluetooth::BLEStatus status,
    android::sp<IInterface> callback,
    bluetooth::BluetoothClientInstance* client) {
  VLOG(1) << __func__ << " client ID: " << client->GetClientId()
          << " status: " << status;
  android::sp<IBluetoothGattServerCallback> cb(
      static_cast<IBluetoothGattServerCallback*>(callback.get()));
  cb->OnServerRegistered(
      status,
      (status == bluetooth::BLE_STATUS_SUCCESS) ?
          client->GetClientId() : kInvalidClientId);
}

}  // namespace binder
}  // namespace ipc
