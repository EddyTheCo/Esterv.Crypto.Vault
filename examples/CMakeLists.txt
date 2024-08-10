if(USE_QML AND BUILD_EXAMPLES)
  if(EMSCRIPTEN OR ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
  endif(EMSCRIPTEN OR ANDROID)
  foreach(example "simple")
    qt_add_executable(${example} ${example}.cpp)

    qt6_add_qml_module(
      ${example}
      URI
      E${example}
      VERSION
      1.0
      QML_FILES
      "qml/${example}.qml"
      RESOURCE_PREFIX
      "/esterVtech.com/imports"
      IMPORT_PATH
      ${CMAKE_BINARY_DIR})
    target_link_libraries(
      ${example}
      PRIVATE
        vault
        $<$<STREQUAL:$<TARGET_PROPERTY:vault,TYPE>,STATIC_LIBRARY>:vaultplugin>)

    set_target_properties(${example} PROPERTIES WIN32_EXECUTABLE ON
                                                MACOSX_BUNDLE ON)
    install(
      TARGETS ${example}
      COMPONENT ${PROJECT_NAME}-examples
      BUNDLE DESTINATION . COMPONENT ${PROJECT_NAME}-examples
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    if(QTDEPLOY)
      qt_generate_deploy_qml_app_script(TARGET ${example} OUTPUT_SCRIPT
                                        deploy_script)
      install(SCRIPT ${deploy_script} COMPONENT ${PROJECT_NAME}-examples)
    endif(QTDEPLOY)
    if(ANDROID)
      set_property(
        TARGET ${example}
        APPEND
        PROPERTY QT_ANDROID_MIN_SDK_VERSION 30)
      set_property(
        TARGET ${example}
        APPEND
        PROPERTY QT_ANDROID_TARGET_SDK_VERSION 34)
      set_property(
        TARGET ${example}
        APPEND
        PROPERTY QT_ANDROID_SDK_BUILD_TOOLS_REVISION 34.0.0)

    endif(ANDROID)
    if(EMSCRIPTEN OR ANDROID)
      FetchContent_Declare(
        EstervDesigns
        GIT_REPOSITORY https://github.com/EddyTheCo/Esterv.Designs.git
        GIT_TAG develop
        FIND_PACKAGE_ARGS 1.2 COMPONENTS FlatControl CONFIG)
      FetchContent_MakeAvailable(EstervDesigns)
      target_link_libraries(
        ${example}
        PRIVATE
          Esterv::FlatControl
          Qt6::QuickControls2
          $<$<STREQUAL:$<TARGET_PROPERTY:Esterv::FlatControl,TYPE>,STATIC_LIBRARY>:Esterv::FlatControlplugin>
      )
      target_compile_definitions(${example}
                                 PRIVATE FORCE_STYLE="Esterv.Controls.Flat")
    endif(EMSCRIPTEN OR ANDROID)
  endforeach()
endif(USE_QML AND BUILD_EXAMPLES)
