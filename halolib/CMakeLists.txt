set(LibraryVersion "1.0")
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

# Use REST's library convention for dictionaries, headers and rest-config.
COMPILELIB("")

if (TEST)
    foreach (halo_test TRestHaloEvent TRestHaloTrimProcess TRestHaloCombine)
        add_executable(${halo_test}_test test/${halo_test}_test.cxx)
        target_include_directories(${halo_test}_test
                                   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
        target_link_libraries(${halo_test}_test PRIVATE ${libname})

        # These tests use assert for their checks, including in Release builds.
        if (MSVC)
            target_compile_options(${halo_test}_test PRIVATE /UNDEBUG)
        else ()
            target_compile_options(${halo_test}_test PRIVATE -UNDEBUG)
        endif ()

        add_test(NAME ${halo_test}_test COMMAND ${halo_test}_test)
    endforeach ()
endif ()
