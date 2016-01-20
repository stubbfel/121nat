file(GLOB_RECURSE 121Nat_Src_Files "*.h" "*.cpp")
add_executable(121Nat ${121Nat_Src_Files})
target_link_libraries (121Nat pthread  ${CMAKE_SOURCE_DIR}/lib/bin/libtins.so ${CMAKE_SOURCE_DIR}/lib/bin/libjsoncpp.a)
set_property(TARGET 121Nat PROPERTY CXX_STANDARD 11)
set_property(TARGET 121Nat PROPERTY CXX_STANDARD_REQUIRED ON)
install(TARGETS 121Nat
        RUNTIME DESTINATION bin
)

