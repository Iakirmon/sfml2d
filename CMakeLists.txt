cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE "C:/game2d/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(Platformer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ścieżka do SFML
set(SFML_DIR "C:/game2d/vcpkg/vcpkg_installed/x64-windows/share/sfml")

find_package(SFML COMPONENTS graphics window system audio)

add_executable(Platformer
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Platform.cpp
    src/Coin.cpp
    src/Level.cpp
    src/Animator.cpp
    src/AudioManager.cpp
    src/ParallaxLayer.cpp
)

target_link_libraries(Platformer PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)

add_custom_command(TARGET Platformer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Platformer>/assets
)

set(VCPKG_BIN "C:/game2d/vcpkg/vcpkg_installed/x64-windows/bin")

# Copy SFML DLLs – zawsze Release DLL (działają dla Release i RelWithDebInfo).
# Dla Debug: użyj cmake --build . --config RelWithDebInfo (breakpointy działają,
# brak problemu z -d DLL).
add_custom_command(TARGET Platformer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/sfml-window-2.dll"   $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/sfml-graphics-2.dll" $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/sfml-system-2.dll"   $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/sfml-audio-2.dll"    $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/openal32.dll"        $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/ogg.dll"             $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/vorbis.dll"          $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/vorbisenc.dll"       $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/vorbisfile.dll"      $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/fmt.dll"             $<TARGET_FILE_DIR:Platformer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_BIN}/freetype.dll"        $<TARGET_FILE_DIR:Platformer>
)
