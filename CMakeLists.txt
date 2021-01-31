cmake_minimum_required(VERSION 3.19)
project(BPRoadRuleLimitRemover CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
	${SOURCES}
	main.cpp
	Ui/RRLimitRemover.cpp
	)

set(HEADERS
	${HEADERS}
	Ui/RRLimitRemover.h
	)

set(RESOURCES
	${RESOURCES}
	Ui/RRLimitRemover.qrc
	)

set(UIS
	${UIS}
	Ui/RRLimitRemover.ui
	)

# Use if Qt6_DIR isn't set in PATH
#set(Qt6_DIR C:\\Qt\\6.0.0\\msvc2019_64\\lib\\cmake\\Qt6)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(APP_ICON_RESOURCE_WINDOWS "${ROOT}/Ui/Icon.rc")

add_executable(BPRoadRuleLimitRemover WIN32 ${SOURCES} ${HEADERS} ${RESOURCES} ${UIS} ${APP_ICON_RESOURCE_WINDOWS})

# Qt
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# libbinaryio
add_subdirectory(3rdparty/libbinaryio "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libbinaryio" EXCLUDE_FROM_ALL)
target_include_directories(BPRoadRuleLimitRemover PRIVATE ${ROOT} "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libbinaryio" EXCLUDE_FROM_ALL)

target_link_libraries(BPRoadRuleLimitRemover PRIVATE libbinaryio Qt6::Core Qt6::Gui Qt6::Widgets)

# VS stuff
set_property(DIRECTORY ${ROOT} PROPERTY VS_STARTUP_PROJECT BPRoadRuleLimitRemover)