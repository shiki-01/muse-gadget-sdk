/* Mugi: ホストテスト用の esp_eap_client.h の代用品。呼び出しを記録する。 */
#pragma once

#include <stdbool.h>

#include "esp_err.h"

typedef enum { ESP_EAP_TYPE_PEAP = 4 } esp_eap_method_t;

void mugi_fake_log(const char *fmt, ...);

esp_err_t esp_wifi_sta_enterprise_enable(void);
esp_err_t esp_wifi_sta_enterprise_disable(void);
esp_err_t esp_eap_client_set_identity(const unsigned char *v, int len);
esp_err_t esp_eap_client_set_username(const unsigned char *v, int len);
esp_err_t esp_eap_client_set_password(const unsigned char *v, int len);
esp_err_t esp_eap_client_set_eap_methods(esp_eap_method_t m);
esp_err_t esp_eap_client_use_default_cert_bundle(bool use);
esp_err_t esp_eap_client_set_domain_name(const char *d);
esp_err_t esp_eap_client_set_disable_time_check(bool d);
