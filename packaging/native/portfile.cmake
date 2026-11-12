vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/titanfyre/Packet-Capture-SDK/releases/download/sdk-v0.1.1/packet-capture-sdk-sdk-0.1.1.tar.gz"
    FILENAME "packet-capture-sdk-sdk-0.1.1.tar.gz"
    SHA512 9919d8bc6ce700028f05a8518f007692e7b60ced0d6db929bee02758ad7c0f8af9b7e9fc575e05bd2b8ceef043fe97d3cbbdd8ab31eb27530423be472b4fd7ca
)
vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE "${ARCHIVE}" NO_REMOVE_ONE_LEVEL)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME PacketCaptureSDK CONFIG_PATH lib/cmake/PacketCaptureSDK)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
