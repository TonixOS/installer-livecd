project(cloudos-livecd)
cmake_minimum_required(VERSION 2.8)
set( CMAKE_VERBOSE_MAKEFILE on )
#set( CMAKE_CXX_FLAGS "-fsanitize=address -std=c++11" )
set( CMAKE_CXX_FLAGS "-std=c++11" )
set( CMAKE_BUILD_TYPE debug )
add_definitions(-DBOOST_NO_DEPRECATED)
add_definitions(-DBOOST_NO_CXX11_SCOPED_ENUMS)
#add_definitions(-Weverything)
#add_definitions(-Wno-c++98-compat)
#add_definitions(-Wno-padded)

# we need this flag, else all linking tools will fail to link...
# if not defined, some thing like this will be shown:
#   libcloudos.so: undefined reference to `boost::log::v2s_mt_posix::aux::once_block_sentry::enter_once_block() const'
add_definitions(-DBOOST_ALL_DYN_LINK)

include_directories ("${PROJECT_SOURCE_DIR}")

add_executable(cloudos-livecd main.cpp InstallerLiveCD.cpp)

target_link_libraries(cloudos-livecd cloudos
                                     boost_filesystem
				     boost_system
				     boost_log
				     boost_log_setup
				     boost_thread
				     boost_regex
				     boost_system
				     protobuf
				     pthread
		     )

install(TARGETS cloudos-livecd RUNTIME DESTINATION bin)
