cmake_minimum_required( VERSION 3.5 )

project(proton)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract(rps rps
   ${CMAKE_CURRENT_SOURCE_DIR}/src/rps.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/transfer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/balance.cpp
)

target_include_directories(rps
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(rps PRIVATE -D_LIBCPP_STRSTREAM -DBOOST_EXCEPTION_DISABLE=1 -D_GLIBCXX_SSTREAM=1 -D_LIBCPP_SSTREAM -DBOOST_NO_IOSTREAM -DBOOST_NO_STRINGSTREAM )

set_target_properties(rps
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

target_compile_options( rps PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian)