set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(OPENCR_IMAGE STREQUAL "bootloader")
    set(LIBXR_SYSTEM none)
else()
    set(LIBXR_SYSTEM FreeRTOS)
endif()
set(LIBXR_DRIVER st)
set(XROBOT_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Modules)

add_subdirectory(Middlewares/Third_Party/LibXR)

target_compile_features(xr PUBLIC cxx_std_20)
target_link_libraries(xr PUBLIC stm32cubemx)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(xr
    PUBLIC $<TARGET_PROPERTY:stm32cubemx,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC Core/Inc
    PUBLIC User
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    PUBLIC $<TARGET_PROPERTY:xr,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC User
)

target_link_libraries(${CMAKE_PROJECT_NAME} stm32cubemx xr)
