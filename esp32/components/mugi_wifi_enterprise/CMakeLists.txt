# Mugi: WPA2-Enterprise (PEAP/MSCHAPV2) 接続の追加コンポーネント。
# upstream のファイルへの変更を最小にするため、独立したコンポーネントにしている。
set(srcs "mugi_wifi_enterprise.c")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi wpa_supplicant log
)

# CA 証明書を指定した場合は、固定のシンボル名で埋め込む
# (_binary_mugi_eap_ca_pem_start / _end)。未指定なら esp_crt_bundle を使う。
if(CONFIG_MUGI_EAP_CA_CERT_PATH)
    get_filename_component(mugi_ca_path "${CONFIG_MUGI_EAP_CA_CERT_PATH}"
                           ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    if(NOT EXISTS "${mugi_ca_path}")
        message(FATAL_ERROR "CONFIG_MUGI_EAP_CA_CERT_PATH not found: ${mugi_ca_path}")
    endif()
    configure_file("${mugi_ca_path}" "${CMAKE_CURRENT_BINARY_DIR}/mugi_eap_ca.pem" COPYONLY)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${mugi_ca_path}")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE MUGI_EAP_CA_EMBEDDED=1)
    target_add_binary_data(${COMPONENT_LIB} "${CMAKE_CURRENT_BINARY_DIR}/mugi_eap_ca.pem" TEXT)
endif()
