cmake_minimum_required(VERSION 3.16)
project(FAANG_LLD_Design_Patterns_Mastery CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -pthread)
endif()

# Creational Patterns
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/creational/factory_method/notification_dispatcher.cpp")
    add_executable(factory_method_notification creational/factory_method/notification_dispatcher.cpp)
endif()

# Structural Patterns
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/decorator/api_gateway_pipeline.cpp")
    add_executable(decorator_api_gateway structural/decorator/api_gateway_pipeline.cpp)
endif()

# Behavioral Patterns
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/strategy/dynamic_pricing.cpp")
    add_executable(strategy_dynamic_pricing behavioral/strategy/dynamic_pricing.cpp)
endif()
