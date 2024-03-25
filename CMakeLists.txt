cmake_minimum_required(VERSION 3.24)
option(USE_QML "Create QML types and modules" ON)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
	ccommon
	GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
	GIT_TAG use_IFW
	)
FetchContent_MakeAvailable(ccommon)
version_from_git(
	LOG       OFF
	TIMESTAMP "%Y%m%d%H%M%S"
	)
project(QtVault VERSION ${VERSION} DESCRIPTION "Store encrypted data on the local system" LANGUAGES CXX)

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
		endif(NOT openssl_POPULATED)
		include(${openssl_SOURCE_DIR}/openssl.cmake)
		create_targets()
	endif(EMSCRIPTEN OR ANDROID )
endif(NOT OPENSSL_FOUND)

if(OPENSSL_FOUND AND TARGET OpenSSL::Crypto AND Qt6_FOUND)
	qt_standard_project_setup()
	add_library(vault include/vault.hpp src/vault.cpp)
	add_library(${PROJECT_NAME}::vault ALIAS vault)
	set_target_properties(vault PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_MAJOR})
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


	target_include_directories(vault PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Esterv/>")

	target_link_libraries(vault PUBLIC Qt6::Core Qt6::Network OpenSSL::Crypto)
	target_compile_definitions(vault PRIVATE WINDOWS_QVAULT)
	if(EMSCRIPTEN)
		target_compile_definitions(vault PRIVATE USE_EMSCRIPTEN)
	endif(EMSCRIPTEN)

	if(NOT EMSCRIPTEN AND NOT ANDROID)
		add_executable(QVaultCli src/QVaultCli.cpp)
		target_link_libraries(QVaultCli PRIVATE vault)
		add_executable(${PROJECT_NAME}::QVaultCli ALIAS QVaultCli)
		set_target_properties(QVaultCli PROPERTIES
			WIN32_EXECUTABLE ON
			MACOSX_BUNDLE ON
			)
		install(TARGETS QVaultCli
			BUNDLE DESTINATION .
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
			COMPONENT vaultCli
			)
		qt_generate_deploy_app_script(
			TARGET QVaultCli
			OUTPUT_SCRIPT deploy_script
			)
		install(SCRIPT ${deploy_script})
	endif(NOT EMSCRIPTEN AND NOT ANDROID)


	install(TARGETS vault
		EXPORT ${PROJECT_NAME}-config
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT vault
		)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Esterv
		COMPONENT vault
		)

	install(EXPORT ${PROJECT_NAME}-config
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
		NAMESPACE ${PROJECT_NAME}::
		COMPONENT ${PROJECT_NAME}
		)
	include(CMakePackageConfigHelpers)
	configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
		INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
		)
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
		VERSION ${VERSION}
		COMPATIBILITY SameMajorVersion
		)
	install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
		COMPONENT vault
		)
	export(EXPORT ${PROJECT_NAME}-config
		FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
		)
	if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
		add_subdirectory(examples)
		set(cpack_file_name "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
		if(NOT BUILD_SHARED_LIBS)
			set(cpack_file_name "${cpack_file_name}-static")
		endif(NOT BUILD_SHARED_LIBS)
		set(CPACK_PACKAGE_FILE_NAME ${cpack_file_name})
		include(CPack)
		include(CPackIFW)
		cpack_add_component(vault)
		cpack_ifw_configure_component(vault LICENSES "License" ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
		cpack_ifw_add_repository("Repo" URL "${REPO_URL}/packages/${CMAKE_SYSTEM_NAME}/" )
	endif(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

endif(OPENSSL_FOUND AND TARGET OpenSSL::Crypto AND Qt6_FOUND)


if(BUILD_DOCS)
	build_docs()
endif()
