cmake_minimum_required(VERSION 3.18)
project(EcoVillage VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# optionnel si vous utilisez vcpkg
# set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")


find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)
find_package(nlohmann_json REQUIRED)


add_executable(ecovillage
src/main.cpp
# ajouter d'autres fichiers .cpp ici ou via target_sources
)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

target_include_directories(ecovillage PRIVATE src)


target_link_libraries(ecovillage PRIVATE sfml-system sfml-window sfml-graphics sfml-audio nlohmann_json::nlohmann_json)


# ImGui : si vous intégrez ImGui manuellement, linkez les fichiers ici.