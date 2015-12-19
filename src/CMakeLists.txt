set(CMAKE_INCLUDE_CURRENT_DIR on)
include_directories(../thirdparty)

add_executable (topkube-format-problems
format_problems.cc
rank_parser.cc
scanner.cc
rank.cc
topk_algorithms.cc)

add_executable (topkube-benchmark
benchmark.cc
rank_parser.cc
scanner.cc
rank.cc
topk_algorithms.cc)

target_link_libraries (topkube-benchmark ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries(nanocubes-app PUBLIC
# polycover_core
# lluitk_core
# jsoncpp
# ${GLFW_LIBRARIES}
# ${FREEIMAGE_LIBRARIES}
# ${Boost_LIBRARIES}
# ${CURL_LIBRARIES}
# ${ANT_TWEAK_BAR_LIBRARY}
# ${NANOMSG_LIBRARY})

install_targets(/bin topkube-format-problems topkube-benchmark)