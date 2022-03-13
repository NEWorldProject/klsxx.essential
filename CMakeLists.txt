cmake_minimum_required(VERSION 3.11)

project(KLSXX)

kls_add_library_module(kls.essential klsxx::essential)
kls_public_source_directory(kls.essential Published)
kls_module_source_directory(kls.essential Module)
