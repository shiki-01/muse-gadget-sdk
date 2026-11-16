/* Mugi: ホストテスト用の esp_log.h の代用品。ログは捨てる。 */
#pragma once

#include "esp_err.h"

#define ESP_LOGE(tag, fmt, ...) ((void)(tag))
#define ESP_LOGW(tag, fmt, ...) ((void)(tag))
#define ESP_LOGI(tag, fmt, ...) ((void)(tag))
