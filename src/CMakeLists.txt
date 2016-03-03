file(GLOB_RECURSE 121Nat_Src_Files "*.h" "*.cpp")
INCLUDE_DIRECTORIES(../lib/src/libtins/include)
INCLUDE_DIRECTORIES(../lib/src/jsoncpp/include)
add_executable(121Nat ${121Nat_Src_Files})
target_link_libraries (121Nat pthread tins jsoncpp_lib_static)
set_property(TARGET 121Nat PROPERTY CXX_STANDARD 11)
set_property(TARGET 121Nat PROPERTY CXX_STANDARD_REQUIRED ON)
install(TARGETS 121Nat
        RUNTIME DESTINATION bin
)

