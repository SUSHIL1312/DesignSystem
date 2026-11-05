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

# ==============================================================================
# Creational Patterns
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/creational/factory_method/notification_dispatcher.cpp")
    add_executable(creational_factory_method creational/factory_method/notification_dispatcher.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/creational/abstract_factory/cloud_infrastructure_provisioner.cpp")
    add_executable(creational_abstract_factory creational/abstract_factory/cloud_infrastructure_provisioner.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/creational/builder/distributed_query_builder.cpp")
    add_executable(creational_builder creational/builder/distributed_query_builder.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/creational/singleton/threadsafe_telemetry_registry.cpp")
    add_executable(creational_singleton creational/singleton/threadsafe_telemetry_registry.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/creational/prototype/lambda_sandbox_cloner.cpp")
    add_executable(creational_prototype creational/prototype/lambda_sandbox_cloner.cpp)
endif()

# ==============================================================================
# Structural Patterns
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/adapter/legacy_banking_payment_adapter.cpp")
    add_executable(structural_adapter structural/adapter/legacy_banking_payment_adapter.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/decorator/api_gateway_pipeline.cpp")
    add_executable(structural_decorator structural/decorator/api_gateway_pipeline.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/facade/one_click_checkout_facade.cpp")
    add_executable(structural_facade structural/facade/one_click_checkout_facade.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/composite/google_drive_filesystem.cpp")
    add_executable(structural_composite structural/composite/google_drive_filesystem.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/proxy/secure_document_cache_proxy.cpp")
    add_executable(structural_proxy structural/proxy/secure_document_cache_proxy.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/bridge/cross_platform_video_player.cpp")
    add_executable(structural_bridge structural/bridge/cross_platform_video_player.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/structural/flyweight/geospatial_tile_renderer.cpp")
    add_executable(structural_flyweight structural/flyweight/geospatial_tile_renderer.cpp)
endif()

# ==============================================================================
# Behavioral Patterns
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/strategy/dynamic_pricing.cpp")
    add_executable(behavioral_strategy behavioral/strategy/dynamic_pricing.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/observer/realtime_collaborative_presence.cpp")
    add_executable(behavioral_observer behavioral/observer/realtime_collaborative_presence.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/command/cloud_ops_rollback_engine.cpp")
    add_executable(behavioral_command behavioral/command/cloud_ops_rollback_engine.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/state/order_lifecycle_state_machine.cpp")
    add_executable(behavioral_state behavioral/state/order_lifecycle_state_machine.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/chain_of_responsibility/api_rate_limit_security_waf.cpp")
    add_executable(behavioral_chain_of_responsibility behavioral/chain_of_responsibility/api_rate_limit_security_waf.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/iterator/social_graph_bfs_iterator.cpp")
    add_executable(behavioral_iterator behavioral/iterator/social_graph_bfs_iterator.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/template_method/datalake_etl_ingestion.cpp")
    add_executable(behavioral_template_method behavioral/template_method/datalake_etl_ingestion.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/behavioral/mediator/video_conferencing_mediator.cpp")
    add_executable(behavioral_mediator behavioral/mediator/video_conferencing_mediator.cpp)
endif()
