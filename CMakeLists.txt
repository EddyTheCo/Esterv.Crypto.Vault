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

include(FindOpenSSL)

if(OPENSSL_FOUND AND TARGET OpenSSL::Crypto )
	qt_standard_project_setup()

	add_library(vault include/vault.hpp src/vault.cpp)
        target_include_directories(vault PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            "$<INSTALL_INTERFACE:/Esterv/${PROJECT_NAME}>")

        target_link_libraries(vault PUBLIC Qt6::Core Qt6::Network OpenSSL::Crypto)

        add_executable(QVaultCli src/QVaultCli.cpp)
        target_link_libraries(QVaultCli PRIVATE vault)

        add_library(${PROJECT_NAME}::vault ALIAS vault)
        add_executable(${PROJECT_NAME}::QVaultCli ALIAS QVaultCli)

endif()


