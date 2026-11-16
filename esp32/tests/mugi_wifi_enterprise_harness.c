/* Mugi: mugi_wifi_enterprise.c を IDF なしで検証するハーネス。 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "esp_eap_client.h"
#include "mugi_wifi_enterprise.h"

static char s_log[1024];

void mugi_fake_log(const char *fmt, ...) {
    size_t n = strlen(s_log);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(s_log + n, sizeof(s_log) - n, fmt, ap);
    va_end(ap);
}

esp_err_t esp_wifi_sta_enterprise_enable(void) { mugi_fake_log("enable;"); return ESP_OK; }
esp_err_t esp_wifi_sta_enterprise_disable(void) { mugi_fake_log("disable;"); return ESP_OK; }
esp_err_t esp_eap_client_set_identity(const unsigned char *v, int len) { mugi_fake_log("identity=%.*s;", len, v); return ESP_OK; }
esp_err_t esp_eap_client_set_username(const unsigned char *v, int len) { mugi_fake_log("user=%.*s;", len, v); return ESP_OK; }
/* パスワードは値そのものではなく長さだけを記録する */
esp_err_t esp_eap_client_set_password(const unsigned char *v, int len) { (void)v; mugi_fake_log("passlen=%d;", len); return ESP_OK; }
esp_err_t esp_eap_client_set_eap_methods(esp_eap_method_t m) { mugi_fake_log("methods=%d;", (int)m); return ESP_OK; }
esp_err_t esp_eap_client_use_default_cert_bundle(bool u) { mugi_fake_log("bundle=%d;", u); return ESP_OK; }
esp_err_t esp_eap_client_set_domain_name(const char *d) { mugi_fake_log("domain=%s;", d); return ESP_OK; }
esp_err_t esp_eap_client_set_disable_time_check(bool d) { mugi_fake_log("timecheck_off=%d;", d); return ESP_OK; }

static int s_fail;

static void expect(const char *name, const char *want) {
    if (strcmp(s_log, want) != 0) {
        printf("FAIL %s\n  want: %s\n  got:  %s\n", name, want, s_log);
        s_fail = 1;
    }
    s_log[0] = '\0';
}

int main(void) {
#if CONFIG_MUGI_WIFI_ENTERPRISE
    esp_err_t err;
    const char *ssid = mugi_wifi_enterprise_ssid();
    if (!ssid || strcmp(ssid, "corp-net") != 0) { printf("FAIL ssid\n"); return 1; }

    /* 対象外の SSID では何も設定しない */
    err = mugi_wifi_enterprise_prepare("home-wifi");
    if (err != ESP_OK) { printf("FAIL other ssid err\n"); s_fail = 1; }
    expect("other ssid", "");

#ifdef TEST_NOPASS
    /* パスワードが空なら有効化せずエラーを返し、中途半端な設定を残さない */
    err = mugi_wifi_enterprise_prepare("corp-net");
    if (err != ESP_ERR_INVALID_STATE) { printf("FAIL nopass err\n"); s_fail = 1; }
    expect("nopass", "disable;");
    if (!s_fail) printf("OK\n");
    return s_fail;
#endif

    /* 対象の SSID では認証情報を設定して有効化する */
    err = mugi_wifi_enterprise_prepare("corp-net");
    if (err != ESP_OK) { printf("FAIL match err\n"); s_fail = 1; }
    expect("match",
#ifdef TEST_ANON
           "identity=anon@example.org;user=alice;passlen=8;methods=4;bundle=1;domain=radius.example.org;timecheck_off=1;enable;"
#else
           "identity=alice;user=alice;passlen=8;methods=4;bundle=1;domain=radius.example.org;timecheck_off=1;enable;"
#endif
    );

    /* 有効化済みの状態で別の SSID に変わったら無効化する */
    err = mugi_wifi_enterprise_prepare("home-wifi");
    if (err != ESP_OK) { printf("FAIL back to psk err\n"); s_fail = 1; }
    expect("back to psk", "disable;");

    /* 続けて対象に戻れば、設定し直して有効化する */
    mugi_wifi_enterprise_prepare("corp-net");
    s_log[0] = '\0';
    mugi_wifi_enterprise_prepare("corp-net");
    if (strncmp(s_log, "disable;", 8) != 0) { printf("FAIL reconfigure\n"); s_fail = 1; }
    s_log[0] = '\0';
#else
    if (mugi_wifi_enterprise_ssid() != NULL) { printf("FAIL disabled ssid\n"); return 1; }
    if (mugi_wifi_enterprise_prepare("corp-net") != ESP_OK) { printf("FAIL disabled prepare\n"); s_fail = 1; }
    expect("disabled", "");
#endif
    if (!s_fail) printf("OK\n");
    return s_fail;
}
