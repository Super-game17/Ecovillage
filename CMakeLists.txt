cmake_minimum_required(VERSION 3.20)

# Set this BEFORE project() so vcpkg can affect find_package()
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(EcoVillage VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# <-- utiliser SFML 3 et composants avec la casse correcte -->
find_package(SFML 3 COMPONENTS System Window Graphics Audio REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(TGUI CONFIG REQUIRED)
    
add_executable(ecovillage
    src/main.cpp
)

# exemple: un autre exécutable pour tester un fichier séparé
add_executable(test_file
    tests/test_file.cpp
    src/core/utilitaires.cpp
    src/core/chunk.cpp
    src/core/map.cpp
    src/core/entity.cpp
    src/core/game.cpp
    src/core/spawner.cpp
)

target_include_directories(test_file PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

# Link the imported SFML targets and json (noms exacts)
target_link_libraries(ecovillage PRIVATE
    SFML::System
    SFML::Window
    SFML::Graphics
    SFML::Audio
    nlohmann_json::nlohmann_json
    TGUI::TGUI
)

target_link_libraries(test_file PRIVATE
    SFML::System SFML::Window SFML::Graphics SFML::Audio
    nlohmann_json::nlohmann_json
    TGUI::TGUI
)
