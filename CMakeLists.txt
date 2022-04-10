cmake_minimum_required(VERSION 3.11)

project(KLSXX)

kls_add_library_module(kls.essential klsxx::essential)
kls_public_source_directory(kls.essential Published)
kls_module_source_directory(kls.essential Module)

if (NOT MSVC)
    message("Using fmtlib as a replace for the missing std::format")
    kls_vcpkg_package(fmt)
    find_package(fmt CONFIG REQUIRED)
    target_link_libraries(kls.essential PUBLIC fmt::fmt)
endif()
