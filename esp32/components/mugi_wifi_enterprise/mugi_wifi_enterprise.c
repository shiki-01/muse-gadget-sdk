/*
 * Mugi: WPA2-Enterprise (PEAP/MSCHAPV2) の接続設定。
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "mugi_wifi_enterprise.h"

#include <stdint.h>
#include <string.h>

#include "esp_eap_client.h"
#include "esp_log.h"

// CA 証明書を埋め込むときは CMakeLists.txt が MUGI_EAP_CA_EMBEDDED を定義する。
#ifdef MUGI_EAP_CA_EMBEDDED
extern const uint8_t mugi_eap_ca_pem_start[] asm("_binary_mugi_eap_ca_pem_start");
extern const uint8_t mugi_eap_ca_pem_end[] asm("_binary_mugi_eap_ca_pem_end");
#endif

static const char *TAG = "mugi_eap";

// EAP を有効化済みか。esp_wifi_sta_enterprise_disable() は EAP の設定をすべて
// 消すため、有効化のたびに認証情報を設定し直す。
static bool s_enabled;

const char *mugi_wifi_enterprise_ssid(void) {
#if CONFIG_MUGI_WIFI_ENTERPRISE
    return CONFIG_MUGI_EAP_SSID[0] ? CONFIG_MUGI_EAP_SSID : NULL;
#else
    return NULL;
#endif
}

#if CONFIG_MUGI_WIFI_ENTERPRISE
static esp_err_t configure(void) {
    const char *user = CONFIG_MUGI_EAP_USERNAME;
    const char *pass = CONFIG_MUGI_EAP_PASSWORD;
    // 外側の ID は、指定がなければユーザー名を使う。
    const char *outer = CONFIG_MUGI_EAP_ANON_IDENTITY[0]
                            ? CONFIG_MUGI_EAP_ANON_IDENTITY : user;
    if (!user[0] || !pass[0]) {
        // パスワードは値をログに出さず、未設定であることだけを伝える。
        ESP_LOGE(TAG, "EAP username or password is not set (see devices/sdkconfig.local-ait)");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = esp_eap_client_set_identity((const unsigned char *)outer,
                                                (int)strlen(outer));
    if (err == ESP_OK) {
        err = esp_eap_client_set_username((const unsigned char *)user,
                                          (int)strlen(user));
    }
    if (err == ESP_OK) {
        err = esp_eap_client_set_password((const unsigned char *)pass,
                                          (int)strlen(pass));
    }
    if (err == ESP_OK) err = esp_eap_client_set_eap_methods(ESP_EAP_TYPE_PEAP);

#ifdef MUGI_EAP_CA_EMBEDDED
    if (err == ESP_OK) {
        err = esp_eap_client_set_ca_cert(
            mugi_eap_ca_pem_start,
            (int)(mugi_eap_ca_pem_end - mugi_eap_ca_pem_start));
    }
#else
    if (err == ESP_OK) err = esp_eap_client_use_default_cert_bundle(true);
#endif

    if (err == ESP_OK && CONFIG_MUGI_EAP_SERVER_DOMAIN[0]) {
        err = esp_eap_client_set_domain_name(CONFIG_MUGI_EAP_SERVER_DOMAIN);
    }
#if CONFIG_MUGI_EAP_DISABLE_TIME_CHECK
    if (err == ESP_OK) err = esp_eap_client_set_disable_time_check(true);
#endif
    return err;
}
#endif  // CONFIG_MUGI_WIFI_ENTERPRISE

esp_err_t mugi_wifi_enterprise_prepare(const char *ssid) {
    const char *target = mugi_wifi_enterprise_ssid();
    bool wanted = target && ssid && strcmp(ssid, target) == 0;

    if (s_enabled) {
        // 前回が 802.1X なら、次が別の SSID でも設定し直すので一度無効化する。
        esp_wifi_sta_enterprise_disable();
        s_enabled = false;
    }
    if (!wanted) return ESP_OK;

#if CONFIG_MUGI_WIFI_ENTERPRISE
    esp_err_t err = configure();
    if (err == ESP_OK) err = esp_wifi_sta_enterprise_enable();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "802.1X setup failed for %s: %s", ssid, esp_err_to_name(err));
        esp_wifi_sta_enterprise_disable();
        return err;
    }
    s_enabled = true;
    ESP_LOGI(TAG, "802.1X (PEAP/MSCHAPV2) enabled for %s", ssid);
    return ESP_OK;
#else
    return ESP_OK;
#endif
}
