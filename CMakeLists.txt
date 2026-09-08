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

# -----------------------------------------------------------------------------
# 2. miniaudio (из ветки master/main)
# -----------------------------------------------------------------------------
FetchContent_Declare(
  miniaudio_repo
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(miniaudio_repo)



# -----------------------------------------------------------------------------
# 5. pffft (из ветки master)
# -----------------------------------------------------------------------------
# FetchContent_Declare(
#   pffft_repo
#   GIT_REPOSITORY https://github.com/martonpeter/pffft.git
#   GIT_TAG        master
# )
# FetchContent_MakeAvailable(pffft_repo)

# Создаем свою библиотеку для заголовков miniaudio, dr_libs и stb, 
# указывая CMake, где именно внутри скачанных репозиториев они лежат:
add_library(audio_headers INTERFACE)
target_include_directories(audio_headers INTERFACE 
    ${miniaudio_repo_SOURCE_DIR}
    ${dr_libs_repo_SOURCE_DIR}
    ${stb_repo_SOURCE_DIR}
)

# Собираем PFFFT как статическую библиотеку из скачанных исходников
# add_library(pffft STATIC
#     ${pffft_repo_SOURCE_DIR}/pffft.c
# )
# target_include_directories(pffft PUBLIC 
#     ${pffft_repo_SOURCE_DIR}
# )

target_link_libraries(pet PRIVATE ftxui::screen
    ftxui::dom
    ftxui::component
    audio_headers
    # pffft
    # libopenmpt::openmpt
    )

if(APPLE)
    # macOS требуются системные аудио-фреймворки
    target_link_libraries(MyAudioApp PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
    )
elseif(UNIX AND NOT APPLE)
    # Linux требует системные библиотеки потоков, математики и динамической загрузки
    find_package(Threads REQUIRED)
    target_link_libraries(MyAudioApp PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )
endif()

include(CTest)
enable_testing()

