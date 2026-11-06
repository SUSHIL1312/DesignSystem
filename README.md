# Low-Level Design (LLD) Patterns in C++20

A hands-on practice repository for mastering object-oriented architecture and the Gang of Four (GoF) design patterns using modern C++ idioms.

Each pattern is implemented as a standalone, ready-to-compile `.cpp` problem stub with clear system requirements, interfaces with `// TODO:` markers, and a pre-written test harness with assertions.

---

## Quick Memory Mapper (When to Use What)

Use this quick mental cheat sheet to identify the right design pattern during interviews or system design rounds:

### 1. Intent-to-Pattern Decision Matrix

| Problem / Signal in Requirements | Recommended Pattern | Key Mechanism in C++ |
|:---|:---|:---|
| **Varying algorithms at runtime** (e.g. dynamic pricing, route calculators) | **[Strategy](behavioral/strategy/dynamic_pricing.cpp)** | `std::unique_ptr<IStrategy>` swapped via context setter |
| **Object changes behavior as its internal state changes** (e.g. order lifecycle) | **[State](behavioral/state/order_lifecycle_state_machine.cpp)** | `IState` objects managing context transitions |
| **One-to-many event notification** (e.g. collaborative doc edits, stock tickers) | **[Observer](behavioral/observer/realtime_collaborative_presence.cpp)** | Subscriber list with `subscribe()` / `broadcast()` |
| **Sequential pipeline where any step can reject/short-circuit** (e.g. WAF, auth chain) | **[Chain of Responsibility](behavioral/chain_of_responsibility/api_rate_limit_security_waf.cpp)** | `next_` handler pointer with conditional forwarding |
| **Stacking behaviors dynamically without subclass explosion** (e.g. API middleware) | **[Decorator](structural/decorator/api_gateway_pipeline.cpp)** | Recursive wrapping of `IComponent` via smart pointers |
| **Integrating incompatible legacy or 3rd-party APIs** (e.g. old banking SOAP API) | **[Adapter](structural/adapter/legacy_banking_payment_adapter.cpp)** | Wrapper translating interface calls and data types |
| **Simplifying a complex multi-service subsystem** (e.g. 1-click checkout) | **[Facade](structural/facade/one_click_checkout_facade.cpp)** | Single orchestrator class masking subsystem complexity |
| **Treating individual items and nested trees uniformly** (e.g. file systems) | **[Composite](structural/composite/google_drive_filesystem.cpp)** | Common base where composites delegate to children |
| **Lazy loading, access control, or caching for heavy objects** (e.g. doc media) | **[Proxy](structural/proxy/secure_document_cache_proxy.cpp)** | Same interface; intercepts calls to instantiate/guard real subject |
| **Decoupling abstraction from platform implementation** (e.g. video player UI vs GPU) | **[Bridge](structural/bridge/cross_platform_video_player.cpp)** | Abstraction holds `std::unique_ptr<IImplementor>` bridge |
| **Memory explosion from millions of small objects** (e.g. map tree markers) | **[Flyweight](structural/flyweight/geospatial_tile_renderer.cpp)** | Factory caching immutable intrinsic state via `std::shared_ptr` |
| **Virtual constructor / Open-Closed instantiation** (e.g. notification channels) | **[Factory Method](creational/factory_method/notification_dispatcher.cpp)** | Derived creators override `createProduct()` |
| **Complex object with many optional parameters & validation** (e.g. SQL query) | **[Builder](creational/builder/distributed_query_builder.cpp)** | Method chaining (`.where().limit()`) producing immutable product |
| **Exactly one shared instance with thread-safe access** (e.g. metrics registry) | **[Singleton](creational/singleton/threadsafe_telemetry_registry.cpp)** | Meyers' Singleton (`static T instance;`) + deleted copy/move |
| **Families of coordinated, compatible products** (e.g. multi-cloud stacks) | **[Abstract Factory](creational/abstract_factory/cloud_infrastructure_provisioner.cpp)** | Factory interface producing compatible product suites |
| **Encapsulating requests to support history & rollback** (e.g. cloud ops deployment) | **[Command](behavioral/command/cloud_ops_rollback_engine.cpp)** | Command objects with `execute()` and `undo()` methods |
| **Fixed algorithm skeleton with customizable steps** (e.g. ETL ingestion) | **[Template Method](behavioral/template_method/datalake_etl_ingestion.cpp)** | Public non-virtual template method + protected virtual hooks |
| **Traversing complex structures without exposing internals** (e.g. graph BFS) | **[Iterator](behavioral/iterator/social_graph_bfs_iterator.cpp)** | `hasNext()` / `next()` encapsulating traversal state and cycles |
| **Expensive object creation via snapshot copying** (e.g. microVM sandboxes) | **[Prototype](creational/prototype/lambda_sandbox_cloner.cpp)** | Polymorphic `clone()` returning deep-copied `std::unique_ptr` |
| **Complex $O(N^2)$ communication mesh among components** (e.g. video chat room) | **[Mediator](behavioral/mediator/video_conferencing_mediator.cpp)** | Colleagues reference only central mediator; zero peer coupling |

---

### 2. Disambiguation Cheat Sheet (Common Confusions)

- **Strategy vs State**:
  - *Strategy*: Client explicitly injects the algorithm from the outside. Algorithms are typically independent and stateless.
  - *State*: State changes occur internally based on lifecycle events. State objects often initiate transitions to sibling states.
- **Decorator vs Chain of Responsibility vs Proxy**:
  - *Decorator*: Augments behavior before/after delegation without breaking the call chain; layers can be stacked arbitrarily.
  - *Chain of Responsibility*: Sequential filter pipeline where any node can decide to terminate/short-circuit the request.
  - *Proxy*: Controls access (lazy initialization, permission gating, caching) to a single underlying object rather than adding layers of new behavior.
- **Factory Method vs Abstract Factory vs Builder**:
  - *Factory Method*: A single method creating one product; delegates creation to derived creators.
  - *Abstract Factory*: An interface with multiple factory methods creating an entire family of coordinated products.
  - *Builder*: Focuses on constructing a single complex, step-by-step object with validation and immutability.
- **Adapter vs Facade vs Bridge**:
  - *Adapter*: Retrospective wrapper making two existing, incompatible interfaces work together.
  - *Facade*: Higher-level entry door simplifying a multi-class subsystem; does not define a new abstraction.
  - *Bridge*: Designed upfront to separate an abstraction hierarchy from its implementation backend so both vary independently.
- **Observer vs Mediator**:
  - *Observer*: One-to-many unidirectional broadcast (publisher emits events without knowing subscriber internals).
  - *Mediator*: Many-to-many bidirectional coordinator (peers route all communication through the mediator to avoid direct dependencies).

---

## Recommended Learning Roadmap

If you are preparing for machine coding or system design interviews, tackle the patterns in this order to build your mental models incrementally:

### Phase 1: Encapsulation & Dynamic Behavior (Start Here)
1. **[Strategy: Dynamic Fare Calculation & Pricing Engine](behavioral/strategy/dynamic_pricing.cpp)**
   *Start here:* Strategy is the cleanest introduction to the Open-Closed Principle and composition over inheritance. It isolates algorithm families behind an interface and demonstrates runtime strategy swapping.
2. **[State: Order Lifecycle State Machine](behavioral/state/order_lifecycle_state_machine.cpp)**
   *Next:* Contrast directly with Strategy. While Strategy swaps independent calculation algorithms, State alters behavior based on internal state transitions.
3. **[Observer: Collaborative Document Event Broker](behavioral/observer/realtime_collaborative_presence.cpp)**
   *Next:* Introduces decoupled one-to-many event notifications and dynamic listener lifecycles (subscribe/unsubscribe).
4. **[Chain of Responsibility: Web Application Firewall Filter Chain](behavioral/chain_of_responsibility/api_rate_limit_security_waf.cpp)**
   *Next:* Builds on behavioral flow control with sequential request inspection and early short-circuit termination.

### Phase 2: Structural Composition & Interface Adaptation
5. **[Decorator: API Gateway Middleware Pipeline](structural/decorator/api_gateway_pipeline.cpp)**
   *Next:* Decorator is the structural counterpart to Chain of Responsibility. Learn how to wrap components recursively to attach responsibilities dynamically without subclass explosion.
6. **[Adapter: Legacy Banking Payment Gateway Adapter](structural/adapter/legacy_banking_payment_adapter.cpp)**
   *Next:* Essential real-world pattern for integrating incompatible legacy SDKs into modern target interfaces via composition.
7. **[Facade: One-Click Checkout Subsystem Orchestrator](structural/facade/one_click_checkout_facade.cpp)**
   *Next:* Learn how to simplify complex subsystem interactions behind a unified high-level interface while keeping underlying components accessible.
8. **[Composite: Hierarchical Storage Tree](structural/composite/google_drive_filesystem.cpp)**
   *Next:* Understand recursive tree structures where individual leaf items and composite containers are treated uniformly.

### Phase 3: Object Creation & Lifecycle Management
9. **[Factory Method: Multi-Channel Notification Dispatcher](creational/factory_method/notification_dispatcher.cpp)**
   *Next:* Foundational creational pattern. Learn how virtual constructors delegate concrete product creation, keeping dispatch services open for extension.
10. **[Builder: Distributed SQL Query Builder](creational/builder/distributed_query_builder.cpp)**
    *Next:* Master fluent method chaining, multi-step construction, input validation, and producing immutable products.
11. **[Singleton: Thread-Safe Telemetry Registry](creational/singleton/threadsafe_telemetry_registry.cpp)**
    *Next:* Focus on thread-safe lazy initialization (Meyers' Singleton), deleted copy/move semantics, and unit test isolation.
12. **[Abstract Factory: Multi-Cloud Infrastructure Provisioner](creational/abstract_factory/cloud_infrastructure_provisioner.cpp)**
    *Next:* Elevates Factory Method to coordinated families of interdependent products (Compute, Storage, Network) across cloud providers.

### Phase 4: Advanced Decoupling & Optimization
13. **[Command: Infrastructure Deployment & Rollback Engine](behavioral/command/cloud_ops_rollback_engine.cpp)**
    *Why:* Encapsulate requests as objects with execution history and multi-step `undo()` rollbacks.
14. **[Template Method: Data Lake ETL Ingestion Pipeline](behavioral/template_method/datalake_etl_ingestion.cpp)**
    *Why:* Non-Virtual Interface (NVI) idiom enforcing an invariant algorithm structure with customizable step hooks.
15. **[Proxy: Document Asset Virtual & Protection Proxy](structural/proxy/secure_document_cache_proxy.cpp)**
    *Why:* Virtual proxies (lazy-loading expensive resources) and protection proxies (access control checks).
16. **[Bridge: Cross-Platform Video Player Engine](structural/bridge/cross_platform_video_player.cpp)**
    *Why:* Decouple feature abstractions from implementation backends to avoid $M \times N$ inheritance explosions.
17. **[Flyweight: Geospatial Landmark & POI Render Cache](structural/flyweight/geospatial_tile_renderer.cpp)**
    *Why:* Memory optimization by sharing immutable intrinsic state across thousands of objects.
18. **[Prototype: Serverless Sandbox Cloner](creational/prototype/lambda_sandbox_cloner.cpp)**
    *Why:* Deep copying vs shallow copying and polymorphic cloning from pre-warmed prototypes.
19. **[Iterator: Social Graph BFS Traversal Iterator](behavioral/iterator/social_graph_bfs_iterator.cpp)**
    *Why:* Encapsulate graph traversal state, visited tracking, and cycle prevention behind a clean sequential interface.
20. **[Mediator: Video Conference Room Session Coordinator](behavioral/mediator/video_conferencing_mediator.cpp)**
    *Why:* Replace $O(N^2)$ mesh communication among interacting components with a centralized coordinator.

---

## Problem Index

| Category | Pattern | Problem Link | Real-World Scenario |
|:---|:---|:---|:---|
| **Creational** | Factory Method | [notification_dispatcher.cpp](creational/factory_method/notification_dispatcher.cpp) | Multi-channel alert dispatching (Email, SMS, Push) |
| **Creational** | Abstract Factory | [cloud_infrastructure_provisioner.cpp](creational/abstract_factory/cloud_infrastructure_provisioner.cpp) | Cross-cloud resource provisioning (AWS, GCP, Azure) |
| **Creational** | Builder | [distributed_query_builder.cpp](creational/builder/distributed_query_builder.cpp) | Analytical SQL query construction with validation |
| **Creational** | Singleton | [threadsafe_telemetry_registry.cpp](creational/singleton/threadsafe_telemetry_registry.cpp) | Process-wide concurrent metrics counter registry |
| **Creational** | Prototype | [lambda_sandbox_cloner.cpp](creational/prototype/lambda_sandbox_cloner.cpp) | Serverless microVM execution sandbox cloner |
| **Structural** | Adapter | [legacy_banking_payment_adapter.cpp](structural/adapter/legacy_banking_payment_adapter.cpp) | Integrating legacy banking protocols into modern payment gateways |
| **Structural** | Decorator | [api_gateway_pipeline.cpp](structural/decorator/api_gateway_pipeline.cpp) | API Gateway middleware pipeline (Auth, Rate Limiting, Tracing) |
| **Structural** | Facade | [one_click_checkout_facade.cpp](structural/facade/one_click_checkout_facade.cpp) | Simplifying multi-service checkout coordination |
| **Structural** | Composite | [google_drive_filesystem.cpp](structural/composite/google_drive_filesystem.cpp) | Hierarchical storage tree (Files, Folders, quota calculation) |
| **Structural** | Proxy | [secure_document_cache_proxy.cpp](structural/proxy/secure_document_cache_proxy.cpp) | Virtual lazy loader and access control gatekeeper |
| **Structural** | Bridge | [cross_platform_video_player.cpp](structural/bridge/cross_platform_video_player.cpp) | Decoupling player UI from platform graphics backends |
| **Structural** | Flyweight | [geospatial_tile_renderer.cpp](structural/flyweight/geospatial_tile_renderer.cpp) | Shared 3D landmark mesh/texture cache for map rendering |
| **Behavioral** | Strategy | [dynamic_pricing.cpp](behavioral/strategy/dynamic_pricing.cpp) | Dynamic surge pricing and delivery route cost calculation |
| **Behavioral** | Observer | [realtime_collaborative_presence.cpp](behavioral/observer/realtime_collaborative_presence.cpp) | Collaborative document presence and cursor broker |
| **Behavioral** | Command | [cloud_ops_rollback_engine.cpp](behavioral/command/cloud_ops_rollback_engine.cpp) | Infrastructure deployment pipeline with multi-step rollback |
| **Behavioral** | State | [order_lifecycle_state_machine.cpp](behavioral/state/order_lifecycle_state_machine.cpp) | E-commerce order lifecycle and transition invariants |
| **Behavioral** | Chain of Resp. | [api_rate_limit_security_waf.cpp](behavioral/chain_of_responsibility/api_rate_limit_security_waf.cpp) | Web application firewall request inspection filter chain |
| **Behavioral** | Iterator | [social_graph_bfs_iterator.cpp](behavioral/iterator/social_graph_bfs_iterator.cpp) | Social graph breadth-first traversal iterator |
| **Behavioral** | Template Method | [datalake_etl_ingestion.cpp](behavioral/template_method/datalake_etl_ingestion.cpp) | Invariant data lake ETL ingestion skeleton |
| **Behavioral** | Mediator | [video_conferencing_mediator.cpp](behavioral/mediator/video_conferencing_mediator.cpp) | Video conference room coordinator (Screen share locks, recording alerts) |

---

## Build & Run Setup

### Compiler Requirements
- Compiler: Clang++ (Apple Clang 13+, Clang 14+) or GCC (g++ 11+)
- Standard: C++20 (`-std=c++20`)
- Flags: `-Wall -Wextra -Wpedantic -pthread`

### Direct Compilation
Compile any problem file directly:
```bash
# Example: Strategy
clang++ -std=c++20 -Wall -Wextra -pthread behavioral/strategy/dynamic_pricing.cpp -o bin_strategy
./bin_strategy

# Example: Factory Method
clang++ -std=c++20 -Wall -Wextra -pthread creational/factory_method/notification_dispatcher.cpp -o bin_factory
./bin_factory

# Example: Decorator
clang++ -std=c++20 -Wall -Wextra -pthread structural/decorator/api_gateway_pipeline.cpp -o bin_decorator
./bin_decorator
```

### CMake Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run specific target
./build/behavioral_strategy
./build/creational_factory_method
./build/structural_decorator
```

---

## Design Evaluation Criteria
When implementing and reviewing solutions, focus on:
1. **SOLID Principles**: Single Responsibility (SRP), Open-Closed Principle (OCP via polymorphism/composition), and Dependency Inversion (DIP).
2. **Modern C++ Idioms**:
   - `std::unique_ptr` for exclusive ownership; `std::shared_ptr` only when shared lifetime is genuinely required.
   - Const-correctness and `noexcept` specifications where applicable.
   - Rule of Zero/Five: pure virtual destructors on polymorphic base classes.
   - String views (`std::string_view`) for read-only string parameters.
3. **Thread Safety**: Mutex protection on shared state and avoiding race conditions under concurrent access.
4. **Resilience**: Proper error handling (exceptions vs structured return values) and validation of edge cases.

---

## Additional Practice & References
For further practice on interview-asked Low-Level Design (LLD) and High-Level Design (HLD) questions, you can refer to:
- [System Design Mastery](https://systemdesignmastry.com/) - Practice real-world HLD & LLD interview questions.

