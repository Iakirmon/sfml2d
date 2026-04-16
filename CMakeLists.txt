cmake_minimum_required(VERSION 3.20)
project(Platformer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2 COMPONENTS graphics window system REQUIRED)

add_executable(Platformer
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Platform.cpp
    src/Coin.cpp
    src/Level.cpp
)

target_link_libraries(Platformer PRIVATE sfml-graphics sfml-window sfml-system)

add_custom_command(TARGET Platformer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Platformer>/assets
)
