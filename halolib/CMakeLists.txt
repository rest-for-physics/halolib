project(halolib)

set(LibraryVersion "1.0")
add_definitions(-DLIBRARY_VERSION=\"${LibraryVersion}\")

# Use REST's library build so headers are installed and the library is
# registered with rest-config and the normal REST loader.
COMPILELIB("")

# Copy generated ROOT PCM/rdict files next to the built library so
# ROOT/Cling can find them at runtime when loading the .so from the
# build tree (this makes running PyROOT against the build output work
# without an install step).
# COMPILELIB lists the generated dictionary sources. Derive their PCM paths
# rather than globbing files that do not exist yet on the first configure.
set(_halolib_pcm_files)
foreach(_halolib_dictionary ${files_to_compile_cint})
    string(REGEX REPLACE "\\.cxx$" "_rdict.pcm"
           _halolib_pcm "${_halolib_dictionary}")
    list(APPEND _halolib_pcm_files "${_halolib_pcm}")
endforeach()
if(_halolib_pcm_files)
    add_custom_command(TARGET ${libname} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${libname}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${_halolib_pcm_files}
                        "$<TARGET_FILE_DIR:${libname}>"
        COMMENT "Copying generated ROOT PCM files next to ${libname} library"
    )
endif()

# Ensure the generated PCM files are installed alongside the shared
# library so that an installed libRestHalolib.so can be loaded by ROOT/Cling
# without manual copying.
install(FILES ${_halolib_pcm_files}
                DESTINATION lib
                OPTIONAL)

## --- Tests --------------------------------------------------------------
# Keep assert checks active in Release tests without changing library flags.
if(MSVC)
    set(_halolib_assert_option /UNDEBUG)
else()
    set(_halolib_assert_option -UNDEBUG)
endif()

# Build a small unit test for TRestHaloEvent
if (${TEST} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_executable(TRestHaloEvent_test test/TRestHaloEvent_test.cxx)
    target_include_directories(TRestHaloEvent_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(TRestHaloEvent_test PRIVATE ${libname})
    target_compile_options(TRestHaloEvent_test PRIVATE ${_halolib_assert_option})
    enable_testing()
    add_test(NAME TRestHaloEvent_test COMMAND TRestHaloEvent_test)
endif()

if (${TEST} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_executable(TRestHaloTrimProcess_test test/TRestHaloTrimProcess_test.cxx)
    target_include_directories(TRestHaloTrimProcess_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(TRestHaloTrimProcess_test PRIVATE ${libname})
    target_compile_options(TRestHaloTrimProcess_test PRIVATE ${_halolib_assert_option})
    enable_testing()
    add_test(NAME TRestHaloTrimProcess_test COMMAND TRestHaloTrimProcess_test)
endif()

if (${TEST} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_executable(TRestHaloCombine_test test/TRestHaloCombine_test.cxx)
    target_include_directories(TRestHaloCombine_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(TRestHaloCombine_test PRIVATE ${libname})
    target_compile_options(TRestHaloCombine_test PRIVATE ${_halolib_assert_option})
    enable_testing()
    add_test(NAME TRestHaloCombine_test COMMAND TRestHaloCombine_test)
endif()
