cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.cpp
    src/countdown-widget.hpp
    src/countdown-widget.cpp
    src/widgets/ashmanix-timer.hpp
    src/widgets/ashmanix-timer.cpp
    src/widgets/settings-dialog.hpp
    src/widgets/settings-dialog.cpp
    src/widgets/obs-dock-wrapper.hpp
    src/widgets/ashmanix-timer/timer-ui-manager.cpp
    src/widgets/ashmanix-timer/timer-ui-manager.hpp
    src/widgets/ashmanix-timer/hotkey-manager.cpp
    src/widgets/ashmanix-timer/hotkey-manager.hpp
    src/utils/websocket-notifier.cpp
    src/utils/websocket-notifier.hpp
    src/widgets/ashmanix-timer/timer-persistence.cpp
    src/widgets/ashmanix-timer/timer-persistence.hpp
    src/widgets/settings/colour-change-widget.cpp
    src/widgets/settings/colour-change-widget.hpp
    src/widgets/settings/single-colour-rule-widget.cpp
    src/widgets/settings/single-colour-rule-widget.hpp
    src/classes/colour-rule.cpp
    src/classes/colour-rule.hpp
    src/utils/timer-utils.cpp
    src/utils/timer-utils.hpp
    src/utils/obs-utils.cpp
    src/utils/obs-utils.hpp
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
