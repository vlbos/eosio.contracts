add_contract(ibc ibc src/ibc.cpp)
target_include_directories(ibc.wasm
   PUBLIC
   include)

set_target_properties(ibc.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
