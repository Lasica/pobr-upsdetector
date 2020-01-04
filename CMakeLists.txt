cmake_minimum_required(VERSION 3.0)

project(upsdetector)

find_package(OpenCV REQUIRED
    COMPONENTS
    	opencv_core
    	opencv_imgproc
    	opencv_videoio
    	opencv_highgui
    	opencv_imgcodecs

)


add_executable(upsdetector main.cpp)


target_link_libraries(upsdetector
    PUBLIC
        ${OpenCV_LIBRARIES}
)

install(TARGETS upsdetector RUNTIME DESTINATION bin)
