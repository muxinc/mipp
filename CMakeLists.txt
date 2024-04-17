cmake_minimum_required(VERSION 3.10.2)
include(ExternalProject)
project(mipp)


set(CMAKE_CXX_STANDARD 17)

include_directories(
    /opt/homebrew/include
)

link_directories(
    /opt/homebrew/lib
)

set(SOURCES
    main.cpp
)

add_executable(mipp_test
    src/mipp.cpp
    src/main.cpp
)

add_library(mipp STATIC
    src/mipp.cpp
)

target_sources(mipp PRIVATE
    src/mipp.cpp
)
set_target_properties(mipp PROPERTIES PUBLIC_HEADER
    src/mipp.h
)

install(TARGETS mipp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


target_link_libraries(mipp -lv8 -lv8_libplatform -lcairo)
target_link_libraries(mipp_test mipp)

# SET(ffmpeg_extra_ldflags "-L${CMAKE_BINARY_DIR}")
#  -L/opt/homebrew/lib -lv8 -lv8_libplatform -lcairo")
# ExternalProject_Add(ffmpeg
#     DEPENDS mipp
#     PREFIX ${CMAKE_BINARY_DIR}/ffmpeg
#     URL https://ffmpeg.org/releases/ffmpeg-6.0.tar.gz
#     PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/mipp.patch
#     CONFIGURE_COMMAND <SOURCE_DIR>/configure
#         --enable-mipp
#         --extra-ldflags=${ffmpeg_extra_ldflags}
#         # --extra-cflags="-I/path/to/extra/includes"
# )
