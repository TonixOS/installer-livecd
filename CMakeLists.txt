project(cloudos-livecd)

add_executable(cloudos-livecd main.cpp)

install(TARGETS cloudos-livecd RUNTIME DESTINATION bin)
