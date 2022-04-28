cmake_minimum_required(VERSION 3.9)
project(diploma)
include_directories(include)

configure_file(include/diplomadef.hpp.in ${PROJECT_SOURCE_DIR}/include/diplomadef.hpp)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk)
    set(EOSIOLIB ${EOSIO}/eosiolib)
    add_definitions(-DNOEOS)

    file(GLOB ATOMICASSETS_SRC src/atomicassets/*)
    add_library(atomicassets ${ATOMICASSETS_SRC})
    target_include_directories(atomicassets PUBLIC ${EOSIO}/ ${EOSIOLIB}/core ${EOSIOLIB}/contracts ${EOSIOLIB}/capi ${EOSIOLIB}/native)

    add_library(assets src/assets/asset_repository.cpp include/animals.hpp include/foods.hpp src/assets/food_repository.cpp src/assets/animal_repository.cpp)
    target_include_directories(assets PUBLIC ${EOSIO}/ ${EOSIOLIB}/core ${EOSIOLIB}/contracts ${EOSIOLIB}/capi ${EOSIOLIB}/native)
    target_link_libraries(assets atomicassets)

    add_executable(diploma src/diploma.cpp)
    target_include_directories(diploma PUBLIC ${EOSIO}/ ${EOSIOLIB}/core ${EOSIOLIB}/contracts ${EOSIOLIB}/capi ${EOSIOLIB}/native)

else ()
    find_package(eosio.cdt REQUIRED)

    add_compile_options(-Os)
    add_contract(diploma diploma
            src/diploma.cpp
            src/assets/asset_repository.cpp
            src/assets/animal_repository.cpp
            src/assets/food_repository.cpp
            src/atomicassets/read.cpp
            src/atomicassets/base58_encode.cpp
            src/atomicassets/base58_decode.cpp
            src/atomicassets/zigzag.cpp
            src/atomicassets/atomicdata.cpp
            )
endif ()