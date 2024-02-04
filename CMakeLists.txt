cmake_minimum_required(VERSION 3.24)
option(USE_QML "Create QML types and modules" ON)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
    ccommon
    GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
    GIT_TAG v0.1.1
)
FetchContent_MakeAvailable(ccommon)
version_from_git(
    LOG       OFF
    TIMESTAMP "%Y%m%d%H%M%S"
)
project(QtVault VERSION ${VERSION} DESCRIPTION "Store encrypted and plain data on the system" LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)

if(USE_QML)
    find_package(Qt6 COMPONENTS Core Gui Quick Core Network)
else()
    find_package(Qt6 COMPONENTS Core Network)
endif()


find_package(OpenSSL)


if(NOT OPENSSL_FOUND)
    if(EMSCRIPTEN OR ANDROID)
        if(EMSCRIPTEN)
            set(builturl "https://github.com/EddyTheCo/install-openssl-action/releases/latest/download/openssl-wasm_true-android_none.tar")
        endif(EMSCRIPTEN)
        if(ANDROID)
            set(android_abi_ossl "android-arm")
            if(ANDROID_ABI STREQUAL "arm64-v8a")
                set(android_abi_ossl "android-arm64")
            endif()
            set(builturl "https://github.com/EddyTheCo/install-openssl-action/releases/latest/download/openssl-wasm_false-android_${android_abi_ossl}.tar")
        endif(ANDROID)
        FetchContent_Declare(
            openssl
            DOWNLOAD_EXTRACT_TIMESTAMP true
            URL  ${builturl}
        )
    FetchContent_GetProperties(openssl)
    if(NOT openssl_POPULATED)
        message(STATUS "openssl prebuilt will be downloaded from ${builturl} if not found on the system")
        FetchContent_Populate(openssl)
        message(STATUS "openssl is installed in ${openssl_SOURCE_DIR}")
        set(OPENSSL_FOUND ON)
        add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
        set_target_properties(OpenSSL::Crypto PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${openssl_SOURCE_DIR}/include")

        if(EMSCRIPTEN)
            set_target_properties(OpenSSL::Crypto PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${openssl_SOURCE_DIR}/libx32/libcrypto.a")
        endif(EMSCRIPTEN)
        if(ANDROID)
            set_target_properties(OpenSSL::Crypto PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${openssl_SOURCE_DIR}/lib/libcrypto.a")
        endif(ANDROID)
    endif(NOT openssl_POPULATED)
endif(EMSCRIPTEN)
endif(NOT OPENSSL_FOUND)

if(OPENSSL_FOUND AND TARGET OpenSSL::Crypto AND Qt6_FOUND)
    qt_standard_project_setup()
    add_library(vault include/vault.hpp src/vault.cpp)
    add_library(${PROJECT_NAME}::vault ALIAS vault)
    if(USE_QML)
        qt6_add_qml_module(vault
            URI  Esterv.Utils.Vault
            VERSION 1.0
            RESOURCE_PREFIX
            "/esterVtech.com/imports"
            OUTPUT_TARGETS out_targets_var
            OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/Esterv/Utils/Vault
            IMPORT_PATH ${CMAKE_BINARY_DIR}
        )
    add_library(${PROJECT_NAME}::vaultplugin ALIAS vaultplugin)
    target_link_libraries(vault PUBLIC Qt6::Gui Qt6::Quick)
    target_compile_definitions(vault PRIVATE USE_QML)

    install(TARGETS ${out_targets_var}
        EXPORT ${PROJECT_NAME}-config
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT vault
    )
install(DIRECTORY ${CMAKE_BINARY_DIR}/Esterv/Utils/Vault
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/Esterv/Utils
    COMPONENT vault
)
install(TARGETS vaultplugin
    EXPORT ${PROJECT_NAME}-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/Esterv/Utils/Vault
    COMPONENT vault
)
endif(USE_QML)


target_include_directories(vault PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    "$<INSTALL_INTERFACE:/Esterv/${PROJECT_NAME}>")

target_link_libraries(vault PUBLIC Qt6::Core Qt6::Network OpenSSL::Crypto)
target_compile_definitions(vault PRIVATE WINDOWS_QVAULT)
if(EMSCRIPTEN)
    target_compile_definitions(vault PRIVATE USE_EMSCRIPTEN)
endif(EMSCRIPTEN)

if(NOT EMSCRIPTEN AND NOT ANDROID)
    add_executable(QVaultCli src/QVaultCli.cpp)
    target_link_libraries(QVaultCli PRIVATE vault)
    add_executable(${PROJECT_NAME}::QVaultCli ALIAS QVaultCli)
    install(TARGETS QVaultCli
        EXPORT ${PROJECT_NAME}-config
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT QVaultCli
    )
endif(NOT EMSCRIPTEN AND NOT ANDROID)


install(TARGETS vault
    EXPORT ${PROJECT_NAME}-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT vault
)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Esterv/${PROJECT_NAME}
    COMPONENT vault
)

install(EXPORT ${PROJECT_NAME}-config
    FILE ${PROJECT_NAME}-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
    NAMESPACE ${PROJECT_NAME}::
    COMPONENT ${PROJECT_NAME}
)
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
)
export(EXPORT ${PROJECT_NAME}-config
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(examples)
    set(CPACK_PACKAGE_CONTACT "estervtech")
    include(CTest)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
    include(CPack)
endif()


endif(OPENSSL_FOUND AND TARGET OpenSSL::Crypto AND Qt6_FOUND)


if(BUILD_DOCS)
    get_target_property(build_docs cmake_build_docs SOURCES)
    include(${build_docs})
endif()
