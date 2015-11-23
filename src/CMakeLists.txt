file(GLOB_RECURSE 121Nat_Src_Files "*.h" "*.cpp")
add_executable(121Nat ${121Nat_Src_Files})
target_link_libraries (121Nat pthread  ${CMAKE_SOURCE_DIR}/lib/bin/libtins.so jsoncpp)
target_compile_features(121Nat PRIVATE cxx_range_for)
install(TARGETS 121Nat
        RUNTIME DESTINATION bin
)

