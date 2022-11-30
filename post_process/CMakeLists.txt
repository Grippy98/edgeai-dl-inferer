cmake_minimum_required(VERSION 3.10.0)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

set(POST_PROCESS_SRCS
    src/ti_post_process.cpp
    src/ti_fonts.cpp
    src/ti_post_process_utils.cpp
    src/ti_post_process_image_classification.cpp
    src/ti_post_process_object_detection.cpp
    src/ti_post_process_semantic_segmentation.cpp
    src/ti_post_process_human_pose_estimation.cpp
    )


build_lib(edgeai_post_process # Named argument: library name
          STATIC              # Named argument: library type
          1.0.0               # Named argument: library version
          # The following all will be clubbed under ${ARGN}
          ${POST_PROCESS_SRCS})
