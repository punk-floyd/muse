
add_executable         (test-type-traits test-type-traits.cpp)
target_compile_features(test-type-traits PUBLIC cxx_std_20)
target_link_libraries  (test-type-traits PRIVATE sysrt)
target_link_libraries  (test-type-traits PRIVATE _startup)

add_executable         (test-string test-string.cpp)
target_compile_features(test-string PUBLIC cxx_std_20)
target_link_libraries  (test-string PRIVATE sysrt)
target_link_libraries  (test-string PRIVATE _startup)

add_executable         (test-string_view test-string_view.cpp)
target_compile_features(test-string_view PUBLIC cxx_std_20)
target_link_libraries  (test-string_view PRIVATE sysrt)
target_link_libraries  (test-string_view PRIVATE _startup)

add_executable         (test-charconv test-charconv.cpp)
target_compile_features(test-charconv PUBLIC cxx_std_20)
target_link_libraries  (test-charconv PRIVATE sysrt)
target_link_libraries  (test-charconv PRIVATE _startup)

add_executable         (test-format test-format.cpp)
target_compile_features(test-format PUBLIC cxx_std_20)
target_link_libraries  (test-format PRIVATE sysrt)
target_link_libraries  (test-format PRIVATE _startup)
