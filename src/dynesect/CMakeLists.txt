add_executable(dynesect
    main.cpp
    AIFReader.cpp
    Label.cpp
    Memory.cpp
    RExReader.cpp
)

target_compile_features(dynesect PRIVATE cxx_std_20)
target_link_libraries(dynesect PRIVATE nlohmann_json::nlohmann_json)
dsect_apply_options(dynesect)
