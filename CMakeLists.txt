cmake_minimum_required(VERSION 3.0)

project(upsdetector)

### CONFIGURATION ############################################################
find_package(OpenCV REQUIRED
    COMPONENTS
    opencv_core
    opencv_imgproc
    opencv_videoio
    opencv_highgui
    opencv_imgcodecs
)
###\CONFIGURATION ############################################################

### PROJECT LIBRARY ##########################################################
add_library(upsdetectorlib OBJECT
    maskoperators.cpp
    point.cpp
    segment.cpp
    segmentfiller.cpp
)
# target_include_directories(upsdetectorlib
#     PUBLIC
#     .
# )
target_link_libraries(upsdetectorlib
    PUBLIC
    ${OpenCV_LIBRARIES}
)
###\PROJECT LIBRARY ##########################################################


add_executable(upsdetector main.cpp)
target_link_libraries(upsdetector
    PUBLIC
    upsdetectorlib
    ${OpenCV_LIBRARIES}
)
install(TARGETS upsdetector RUNTIME DESTINATION bin)
