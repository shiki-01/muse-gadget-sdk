/*
 * Mugi: WPA2-Enterprise (PEAP/MSCHAPV2) の接続設定。
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// 設定された 802.1X 用 SSID。無効または未設定なら NULL。
const char *mugi_wifi_enterprise_ssid(void);

// esp_wifi_connect() の直前に呼ぶ。ssid が設定済みの 802.1X 用 SSID なら
// EAP の認証情報を設定して有効化し、そうでなければ有効化済みの EAP を無効化する。
// 設定に失敗したときはエラーを返す（接続は試みない）。
esp_err_t mugi_wifi_enterprise_prepare(const char *ssid);

#ifdef __cplusplus
}
#endif
