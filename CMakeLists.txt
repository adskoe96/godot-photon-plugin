cmake_minimum_required(VERSION 3.22)
project(godot_photon_plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(godot-cpp)

set(SRC_FILES
    src/register_types.cpp
    src/PhotonClient.cpp
    src/PhotonView.cpp
    src/PhotonTransformView.cpp
    src/PhotonRigidbodyView.cpp
    src/PhotonAnimatorView.cpp
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} godot::cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PHOTON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/photon-sdk/linux")
    set(PHOTON_LIBS 
        "${PHOTON_ROOT}/LoadBalancing-cpp/libLoadBalancingRelease64.a" 
        "${PHOTON_ROOT}/Photon-cpp/libPhotonRelease64.a"
        "${PHOTON_ROOT}/Common-cpp/libCommonRelease64.a"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PHOTON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/photon-sdk/windows")
    set(PHOTON_LIBS 
        "${PHOTON_ROOT}/LoadBalancing-cpp/lib/LoadBalancing-cpp_vc17_release_windows_md_x64.lib" 
        "${PHOTON_ROOT}/Photon-cpp/lib/Photon-cpp_vc17_release_windows_md_x64.lib"
        "${PHOTON_ROOT}/Common-cpp/lib/Common-cpp_vc17_release_windows_md_x64.lib"
        "ws2_32.lib"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    "${PHOTON_ROOT}"
)

target_link_libraries(${PROJECT_NAME} ${PHOTON_LIBS})
set(PLUGIN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/addons/photon")

set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX ""
    
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUTPUT_DIR}"
    
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUTPUT_DIR}"
)