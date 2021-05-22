# rank-cpp
FetchContent_Declare(
  rank-cpp
  GIT_REPOSITORY git@github.com:sca-research/rank-cpp.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(rank-cpp)

# CLI11
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG        v1.9.1
)
FetchContent_MakeAvailable(CLI11)

# Run a key rank algorithm
add_executable(rank
    "${CMAKE_CURRENT_SOURCE_DIR}/rank.cpp"
)
target_link_libraries(rank PRIVATE
app_warnings
app_options
  rankcpp
  CLI11
)
target_compile_features(rank PUBLIC cxx_std_17)
target_compile_options(rank PUBLIC -funroll-loops)
set_target_properties(rank PROPERTIES CXX_EXTENSIONS OFF)

# Simulate a CPA
add_executable(sim-cpa
    "${CMAKE_CURRENT_SOURCE_DIR}/simulate-cpa.cpp"
)
target_link_libraries(sim-cpa PRIVATE
  app_warnings
  app_options
  rankcpp
  CLI11
)
target_compile_features(sim-cpa PUBLIC cxx_std_17)
target_compile_options(sim-cpa PUBLIC -funroll-loops)
set_target_properties(sim-cpa PROPERTIES CXX_EXTENSIONS OFF)
