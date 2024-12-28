cmake_minimum_required(VERSION 3.24)

option(USE_QML "Create QML types and modules" ON)
option(BUILD_EXAMPLES "Build the examples" ON)
option(QTDEPLOY "Deploy Qt dependencies for examples" ON)
option(BUILD_DOCS "Build documentation" ON)

include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
  ccommon
  GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
  GIT_TAG v0.5.0)
FetchContent_MakeAvailable(ccommon)
version_from_git(LOG OFF TIMESTAMP "%Y%m%d%H%M%S")

project(
  EstervVault
  VERSION ${VERSION}
  DESCRIPTION "Store encrypted data on the local system"
  LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)
include(InstallRequiredSystemLibraries)

if(USE_QML)
  find_package(Qt6 COMPONENTS Core Gui Quick Core Network)
else()
  find_package(Qt6 COMPONENTS Core Network)
endif()

find_package(OpenSSL)

if(NOT OPENSSL_FOUND)
  if(EMSCRIPTEN OR ANDROID)
    if(EMSCRIPTEN)
      set(builturl
          "https://github.com/EddyTheCo/install-openssl-action/releases/latest/download/openssl-wasm_true-android_none.tar"
      )
    endif(EMSCRIPTEN)
    if(ANDROID)
      set(android_abi_ossl "android-arm")
      if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(android_abi_ossl "android-arm64")
      endif()
      set(builturl
          "https://github.com/EddyTheCo/install-openssl-action/releases/latest/download/openssl-wasm_false-android_${android_abi_ossl}.tar"
      )
    endif(ANDROID)
    FetchContent_Declare(
      openssl
      DOWNLOAD_EXTRACT_TIMESTAMP true
      URL ${builturl})
    FetchContent_GetProperties(openssl)
    if(NOT openssl_POPULATED)
      message(
        STATUS
          "openssl prebuilt will be downloaded from ${builturl} if not found on the system"
      )
      FetchContent_Populate(openssl)
      message(STATUS "openssl is installed in ${openssl_SOURCE_DIR}")
      set(OPENSSL_FOUND ON)
    endif(NOT openssl_POPULATED)
    include(${openssl_SOURCE_DIR}/openssl.cmake)
    create_targets()
  endif(EMSCRIPTEN OR ANDROID)
endif(NOT OPENSSL_FOUND)

if(OPENSSL_FOUND
   AND TARGET OpenSSL::Crypto
   AND Qt6_FOUND)
  qt_standard_project_setup()

  add_library(Vault include/esterv/crypto/vault.hpp src/vault.cpp)
  add_library(Esterv::Vault ALIAS Vault)
  set_target_properties(Vault PROPERTIES VERSION ${VERSION} SOVERSION
                                                            ${VERSION_MAJOR})
  if(USE_QML)
    qt6_add_qml_module(
      Vault
      URI
      Esterv.Crypto.Vault
      VERSION
      1.0
      RESOURCE_PREFIX
      "/esterVtech.com/imports"
      OUTPUT_TARGETS
      out_targets_var
      OUTPUT_DIRECTORY
      ${CMAKE_BINARY_DIR}/Esterv/Crypto/Vault
      IMPORT_PATH
      ${CMAKE_BINARY_DIR})
    add_library(Esterv::Vaultplugin ALIAS Vaultplugin)
    target_link_libraries(Vault PUBLIC Qt6::Gui Qt6::Quick)
    target_compile_definitions(Vault PRIVATE USE_QML)
    target_include_directories(
      Vault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/esterv/crypto)

    install(
      TARGETS ${out_targets_var}
      EXPORT ${PROJECT_NAME}-config
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT ${PROJECT_NAME}-qml
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
              COMPONENT ${PROJECT_NAME}-qml)
    install(
      DIRECTORY ${CMAKE_BINARY_DIR}/Esterv/Crypto/Vault
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/Esterv/Crypto
      COMPONENT ${PROJECT_NAME}-qml)
    install(
      TARGETS Vaultplugin
      EXPORT ${PROJECT_NAME}-config
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/Esterv/Crypto/Vault
      COMPONENT ${PROJECT_NAME}-qml
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
              COMPONENT ${PROJECT_NAME}-qml)
  endif(USE_QML)

  target_include_directories(
    Vault PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                 "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

  target_link_libraries(Vault PUBLIC Qt6::Core Qt6::Network OpenSSL::Crypto)
  target_compile_definitions(Vault PRIVATE WINDOWS_VAULT)
  if(EMSCRIPTEN)
    target_compile_definitions(Vault PRIVATE USE_EMSCRIPTEN)
  endif(EMSCRIPTEN)

  if(NOT EMSCRIPTEN AND NOT ANDROID)
    add_executable(VaultCli src/vault_cli.cpp)
    target_link_libraries(VaultCli PRIVATE Vault)
    set_target_properties(VaultCli PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE
                                                                  ON)
    install(
      TARGETS VaultCli
      EXPORT ${PROJECT_NAME}-config
      COMPONENT ${PROJECT_NAME}-cli
      BUNDLE DESTINATION . COMPONENT ${PROJECT_NAME}-cli)
    if(QTDEPLOY)
      qt_generate_deploy_app_script(TARGET VaultCli OUTPUT_SCRIPT deploy_script)
      install(SCRIPT ${deploy_script} COMPONENT ${PROJECT_NAME}-cli)
    endif(QTDEPLOY)
  endif(NOT EMSCRIPTEN AND NOT ANDROID)

  install(
    TARGETS Vault
    EXPORT ${PROJECT_NAME}-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT ${PROJECT_NAME})
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT ${PROJECT_NAME}-dev)
  build_exports()
  if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(examples)
    build_cpack()
  endif(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

endif(
  OPENSSL_FOUND
  AND TARGET OpenSSL::Crypto
  AND Qt6_FOUND)

if(BUILD_DOCS)
  build_docs()
endif(BUILD_DOCS)
