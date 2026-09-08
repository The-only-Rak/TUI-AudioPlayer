cmake_minimum_required(VERSION 3.25.0)
project(pet VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
add_executable(pet main.cpp Src/Screen.cpp
  Src/Player.cpp)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/ThirdParty" CACHE PATH "")
include(FetchContent)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG        main
)
FetchContent_MakeAvailable(ftxui)


FetchContent_Declare(
  miniaudio_repo
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(miniaudio_repo)






add_library(audio_headers INTERFACE)
target_include_directories(audio_headers INTERFACE 
    ${miniaudio_repo_SOURCE_DIR}
    ${dr_libs_repo_SOURCE_DIR}
    ${stb_repo_SOURCE_DIR}
)



target_link_libraries(pet PRIVATE ftxui::screen
    ftxui::dom
    ftxui::component
    audio_headers

    )

if(APPLE)

    target_link_libraries(MyAudioApp PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
    )
elseif(UNIX AND NOT APPLE)

    find_package(Threads REQUIRED)
    target_link_libraries(MyAudioApp PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )
endif()

include(CTest)
enable_testing()

