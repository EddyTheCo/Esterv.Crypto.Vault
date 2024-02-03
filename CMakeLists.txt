cmake_minimum_required(VERSION 3.24)

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

find_package(Qt6 REQUIRED COMPONENTS Core Network)

find_package(OpenSSL)


if(NOT OPENSSL_FOUND)
    if(EMSCRIPTEN)
        set(builturl "https://github.com/EddyTheCo/install-openssl-action/releases/latest/download/openssl-wasm.tar")
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

              set_target_properties(OpenSSL::Crypto PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${openssl_SOURCE_DIR}/libx32/libcrypto.a")
    endif(NOT openssl_POPULATED)
endif(EMSCRIPTEN)
endif(NOT OPENSSL_FOUND)

if(OPENSSL_FOUND AND TARGET OpenSSL::Crypto )
    qt_standard_project_setup()

    add_library(vault include/vault.hpp src/vault.cpp)
    target_include_directories(vault PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        "$<INSTALL_INTERFACE:/Esterv/${PROJECT_NAME}>")

    target_link_libraries(vault PUBLIC Qt6::Core Qt6::Network OpenSSL::Crypto)
    target_compile_definitions(vault PRIVATE WINDOWS_QVAULT)
    if(EMSCRIPTEN)
        target_compile_definitions(vault PRIVATE USE_EMSCRIPTEN)
    endif(EMSCRIPTEN)

    add_executable(QVaultCli src/QVaultCli.cpp)
    target_link_libraries(QVaultCli PRIVATE vault)

    add_library(${PROJECT_NAME}::vault ALIAS vault)
    add_executable(${PROJECT_NAME}::QVaultCli ALIAS QVaultCli)

    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        add_subdirectory(examples)
    endif(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

endif()


