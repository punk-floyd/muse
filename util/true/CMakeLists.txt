

add_executable         (true true.cpp)
target_compile_features(true PUBLIC cxx_std_20)
target_link_libraries  (true PRIVATE sysrt)
target_link_libraries  (true PRIVATE _startup)
