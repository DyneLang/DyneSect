add_executable(dynepatch main.cpp)

target_compile_features(dynepatch PRIVATE cxx_std_20)
dsect_apply_options(dynepatch)
