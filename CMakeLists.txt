# REST discovers libraries through the CMakeLists.txt at the submodule root.
# Keep the original source layout inside halolib/.
add_subdirectory(halolib)

# Forward COMPILELIB's registration to the framework's source directory.
set(dirs_included ${dirs_included} PARENT_SCOPE)
set(library_added ${library_added} PARENT_SCOPE)
