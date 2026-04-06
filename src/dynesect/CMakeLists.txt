add_executable(dynesect main.cpp)

target_compile_features(dynesect PRIVATE cxx_std_20)
dsect_apply_options(dynesect)
