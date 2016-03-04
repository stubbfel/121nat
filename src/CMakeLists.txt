file(GLOB_RECURSE 121Nat_Src_Files "*.h" "*.cpp")
add_executable(121Nat ${121Nat_Src_Files} ../lib/src/easyloggingpp/src/easylogging++.h)
target_link_libraries (121Nat pthread tins jsoncpp_lib_static)


