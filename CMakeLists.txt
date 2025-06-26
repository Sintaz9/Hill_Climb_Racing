cmake_minimum_required(VERSION 3.16)
project(HillClimbRacing)

set(CMAKE_CXX_STANDARD 17)

# Настройка для 32-битных библиотек
set(BOOST_ARCHITECTURE "-x86")

# Добавляем пути к заголовочным файлам
include_directories(
    "$/libraries/SFML/include"
    "/libraries/BOX2d/include"
    "/include"  # Основная папка с вашими заголовками
)

# Добавляем пути к библиотекам
link_directories(
    "/libraries/SFML/lib"
    "$/libraries/BOX2d/lib/Debug"
)

# Исходные файлы
set(SOURCES
    "/src/main.cpp"
    "/src/Game.cpp"
    "/src/Car.cpp"
    "/src/Menu.cpp"
    "/src/Terrain.cpp"
    "/src/Obstacle.cpp"
    "/src/Camera.cpp"
)

# Создаем исполняемый файл
add_executable(HillClimbRacing ${SOURCES})

# Линкуем SFML и Box2D
target_link_libraries(HillClimbRacing
    sfml-graphics-d
    sfml-window-d
    sfml-system-d 
    sfml-audio-d
    box2dd
)

# Копируем DLL файлы SFML в выходную директорию
add_custom_command(TARGET HillClimbRacing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/libraries/SFML/lib"
    $<TARGET_FILE_DIR:HillClimbRacing>
)