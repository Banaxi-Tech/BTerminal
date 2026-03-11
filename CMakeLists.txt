cmake_minimum_required(VERSION 3.16)

project(BTerminal VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
# On Arch, the GTK4 VTE widget is exposed as vte-2.91-gtk4 via pkg-config.
pkg_check_modules(VTE REQUIRED IMPORTED_TARGET vte-2.91-gtk4)

# Optional: toml++ (header-only). Build falls back to defaults if not present.
include(CheckIncludeFileCXX)
check_include_file_cxx("toml++/toml.h" HAS_TOMLPP)
if(HAS_TOMLPP)
    add_compile_definitions(HAS_TOMLPP)
endif()

add_executable(bterminal
    src/main.cpp
    src/window.cpp
    src/window.h
    src/terminal.cpp
    src/terminal.h
    src/config.cpp
    src/config.h
    src/keybindings.cpp
    src/keybindings.h
    src/colors.cpp
    src/colors.h
    src/config_gui.cpp
    src/config_gui.h
)

target_include_directories(bterminal PRIVATE
    ${GTK4_INCLUDE_DIRS}
    ${VTE_INCLUDE_DIRS}
)

target_link_libraries(bterminal PRIVATE
    PkgConfig::GTK4
    PkgConfig::VTE
)

target_compile_options(bterminal PRIVATE
    ${GTK4_CFLAGS_OTHER}
    ${VTE_CFLAGS_OTHER}
)

target_compile_definitions(bterminal PRIVATE
    BTERMINAL_VERSION="${PROJECT_VERSION}"
    PKGDATADIR="${CMAKE_INSTALL_FULL_DATADIR}/bterminal"
)

# Install binary to /usr/local/bin as requested (absolute path to ignore prefix)
install(TARGETS bterminal RUNTIME DESTINATION /usr/local/bin)

# Install desktop entry to /usr/share/applications
install(FILES resources/bterminal.desktop DESTINATION /usr/share/applications)

# Install auxiliary resources
install(FILES resources/bterminal.css DESTINATION ${CMAKE_INSTALL_DATADIR}/bterminal)
install(FILES resources/icon.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME bterminal.svg)

