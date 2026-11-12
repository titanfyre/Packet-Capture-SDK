cmake_minimum_required(VERSION 3.24)
project(PacketCaptureSDK VERSION 0.1.1 LANGUAGES C CXX)
if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "This SDK requires Windows x64. No driver is installed by this build.")
endif()
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
add_library(TitanPacketCaptureSDK STATIC
  "sdk/SDK/PacketCapture.cpp"
)
add_library(PacketCaptureSDK::sdk ALIAS TitanPacketCaptureSDK)
set_target_properties(TitanPacketCaptureSDK PROPERTIES EXPORT_NAME sdk)
target_compile_features(TitanPacketCaptureSDK PUBLIC cxx_std_17)
target_compile_definitions(TitanPacketCaptureSDK PUBLIC )
target_compile_definitions(TitanPacketCaptureSDK PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
if(MSVC)
  target_compile_options(TitanPacketCaptureSDK PRIVATE /utf-8 /EHsc /W4)
endif()
target_include_directories(TitanPacketCaptureSDK PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sdk>"
  "$<INSTALL_INTERFACE:include/>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sdk/SDK>"
  "$<INSTALL_INTERFACE:include/SDK>"
)
target_link_libraries(TitanPacketCaptureSDK PUBLIC advapi32 comctl32 crypt32 gdi32 newdev ole32 setupapi user32 uuid winhttp wintrust)
install(TARGETS TitanPacketCaptureSDK EXPORT PacketCaptureSDKTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY sdk/ DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl")
install(FILES LICENSE DESTINATION share/packet-capture-sdk)
install(EXPORT PacketCaptureSDKTargets NAMESPACE PacketCaptureSDK:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PacketCaptureSDK)
configure_file("PacketCaptureSDKConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PacketCaptureSDKConfig.cmake" @ONLY)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/PacketCaptureSDKConfigVersion.cmake" VERSION 0.1.1 COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PacketCaptureSDKConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/PacketCaptureSDKConfigVersion.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PacketCaptureSDK)
