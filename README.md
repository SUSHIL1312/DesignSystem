# FAANG LLD & Machine Coding Mastery (C++20)

A deliberate practice repository for Senior Software Engineers (L5/L6 bar) targeting Amazon, Google, and Meta Low-Level Design (LLD) / Machine Coding interviews.

---

## 🏛️ Repository Architecture

```text
.
├── CMakeLists.txt
├── README.md
├── creational/
│   ├── factory_method/
│   │   └── notification_dispatcher.cpp
│   ├── abstract_factory/
│   │   └── cloud_infrastructure_provisioner.cpp
│   ├── builder/
│   │   └── distributed_query_builder.cpp
│   ├── singleton/
│   │   └── threadsafe_telemetry_registry.cpp
│   └── prototype/
│       └── lambda_sandbox_cloner.cpp
├── structural/
│   ├── adapter/
│   │   └── legacy_banking_payment_adapter.cpp
│   ├── decorator/
│   │   └── api_gateway_pipeline.cpp
│   ├── facade/
│   │   └── one_click_checkout_facade.cpp
│   ├── composite/
│   │   └── google_drive_filesystem.cpp
│   ├── proxy/
│   │   └── secure_document_cache_proxy.cpp
│   ├── bridge/
│   │   └── cross_platform_video_player.cpp
│   └── flyweight/
│       └── geospatial_tile_renderer.cpp
└── behavioral/
    ├── strategy/
    │   └── dynamic_pricing.cpp
    ├── observer/
    │   └── realtime_collaborative_presence.cpp
    ├── command/
    │   └── cloud_ops_rollback_engine.cpp
    ├── state/
    │   └── order_lifecycle_state_machine.cpp
    ├── chain_of_responsibility/
    │   └── api_rate_limit_security_waf.cpp
    ├── iterator/
    │   └── social_graph_bfs_iterator.cpp
    ├── template_method/
    │   └── datalake_etl_ingestion.cpp
    └── mediator/
        └── video_conferencing_mediator.cpp
```

---

## 🛠️ Build & Compilation Setup

### Requirements
- **Compiler**: Clang++ (Apple Clang 13+, Clang 14+) or GCC (g++ 11+)
- **Standard**: C++20 (`-std=c++20`)
- **Libraries**: POSIX Threads (`-pthread`), Standard Library (`<iostream>`, `<memory>`, `<string>`, `<vector>`, `<chrono>`, `<cassert>`, `<mutex>`)

### Option A: Direct Compiler Invocations (Fastest feedback loop)
```bash
# Example: Behavioral - Strategy
clang++ -std=c++20 -Wall -Wextra -pthread behavioral/strategy/dynamic_pricing.cpp -o bin_strategy
./bin_strategy

# Example: Structural - Decorator
clang++ -std=c++20 -Wall -Wextra -pthread structural/decorator/api_gateway_pipeline.cpp -o bin_decorator
./bin_decorator

# Example: Creational - Factory Method
clang++ -std=c++20 -Wall -Wextra -pthread creational/factory_method/notification_dispatcher.cpp -o bin_factory
./bin_factory
```

### Option B: CMake Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run specific binary
./build/strategy_dynamic_pricing
./build/decorator_api_gateway
./build/factory_method_notification
```

---

## 🎯 FAANG L5/L6 Evaluation Rubric
When critiquing your solution, evaluation will focus on:
1. **SOLID Principles**: Especially **SRP** (Single Responsibility), **OCP** (Open-Closed via polymorphism/composition), and **DIP** (Dependency Inversion with interfaces).
2. **Modern C++ Idioms**:
   - `std::unique_ptr` vs `std::shared_ptr` (strict ownership semantics vs shared lifecycle).
   - Const-correctness and `noexcept` specifications.
   - Rule of Zero/Three/Five, virtual destructors on abstract base classes.
   - Value semantics, move semantics (`std::move`), and string views (`std::string_view`).
3. **Thread Safety & Concurrency**:
   - Mutex granularities, race conditions, memory visibility.
4. **Resilience & Extensibility**:
   - How easily a 3rd party or new engineer could introduce a new strategy, decorator, or factory without modifying existing client code.
