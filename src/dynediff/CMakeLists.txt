add_executable(dynediff main.cpp)

target_compile_features(dynediff PRIVATE cxx_std_20)
dsect_apply_options(dynediff)
