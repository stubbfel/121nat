cmake_minimum_required(VERSION 3.3.2)
project (121Nat)
#link_directories("$ENV{STAGING_DIR}/usr/lib")
ADD_SUBDIRECTORY(lib/src/jsoncpp)
ADD_SUBDIRECTORY(lib/src/libtins)
ADD_SUBDIRECTORY(src)


