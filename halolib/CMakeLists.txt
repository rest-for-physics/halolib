project(halolib)

set(LibraryVersion "1.0")
add_definitions(-DLIBRARY_VERSION=\"${LibraryVersion}\")

## Use framework COMPILEDIR macro so ROOT dictionaries are generated
## and linked properly for classes using ClassDef/ClassImp.

# Make RestFramework available to be linked by the generated library
set(local_libraries RestFramework)

# Generate dictionary sources, compile sources and create the shared lib
COMPILEDIR(${PROJECT_NAME})

# Copy generated ROOT PCM/rdict files next to the built library so
# ROOT/Cling can find them at runtime when loading the .so from the
# build tree (this makes running PyROOT against the build output work
# without an install step).
set(_halolib_rootdict_dir "${CMAKE_BINARY_DIR}/rootdict")
file(GLOB _halolib_pcm_files
         "${_halolib_rootdict_dir}/CINT_TRestHalo*rdict.pcm"
)
if(_halolib_pcm_files)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${_halolib_pcm_files}
                        "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying generated ROOT PCM files next to ${PROJECT_NAME} library"
    )
endif()

# Ensure the generated PCM files are installed alongside the shared
# library so that an installed libhalolib.so can be loaded by ROOT/Cling
# without manual copying.
install(FILES ${_halolib_pcm_files}
                DESTINATION lib
                OPTIONAL)

## --- Tests --------------------------------------------------------------
# Build a small unit test for TRestHaloEvent
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_executable(TRestHaloEvent_test test/TRestHaloEvent_test.cxx)
    target_include_directories(TRestHaloEvent_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(TRestHaloEvent_test PRIVATE ${PROJECT_NAME})
    enable_testing()
    add_test(NAME TRestHaloEvent_test COMMAND TRestHaloEvent_test)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_executable(TRestHaloTrimProcess_test test/TRestHaloTrimProcess_test.cxx)
    target_include_directories(TRestHaloTrimProcess_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(TRestHaloTrimProcess_test PRIVATE ${PROJECT_NAME})
    enable_testing()
    add_test(NAME TRestHaloTrimProcess_test COMMAND TRestHaloTrimProcess_test)
endif()

