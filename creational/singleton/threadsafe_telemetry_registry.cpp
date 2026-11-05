/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Amazon CloudWatch / Google Monarch Telemetry Registry
 * Category: Creational Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * In high-performance backend microservices at Google or Amazon, a unified, process-wide
 * Telemetry Registry aggregates operational metrics (HTTP request counts, DB error rates, latencies).
 * 
 * Multiple concurrent worker threads simultaneously record metrics. Creating multiple registry instances
 * would cause fragmented counters, incorrect aggregation, and memory bloat.
 * 
 * The system requires:
 *   1. Guaranteed single instance per process lifecycle.
 *   2. Thread-safe, lazy initialization without data races.
 *   3. Prevention of accidental cloning, copying, or assignment.
 *   4. Thread-safe metric updates under intense concurrent load.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Thread-Safe Lazy Initialization:
 *    - Must leverage Modern C++ (C++11/17/20 static local variable initialization guarantee -
 *      "Meyers' Singleton" or `std::call_once`).
 * 2. Uniqueness Guarantees:
 *    - Private constructors; deleted copy constructor, move constructor, and assignment operators.
 * 3. Thread-Safe Data Operations:
 *    - Recording counters and reading aggregated metrics across dozens of threads must be safe from race conditions.
 * 4. Testability / Reset Support:
 *    - Provide a safe `resetForTesting()` hook so automated test suites can reset metrics between runs.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How does Modern C++ guarantee thread-safe static initialization at the language/runtime level?
 * What copy/move operations must be explicitly deleted to prevent duplicate instances?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>
#include <atomic>

class TelemetryRegistry {
private:
    // TODO: Define internal data structures and concurrency synchronization primitives
    std::unordered_map<std::string, uint64_t> counters_;
    mutable std::mutex mtx_;

    // Private constructor enforces single instance
    TelemetryRegistry() = default;
    ~TelemetryRegistry() = default;

public:
    // TODO: Delete copy and move semantics to prevent duplication
    TelemetryRegistry(const TelemetryRegistry&) = delete;
    TelemetryRegistry& operator=(const TelemetryRegistry&) = delete;
    TelemetryRegistry(TelemetryRegistry&&) = delete;
    TelemetryRegistry& operator=(TelemetryRegistry&&) = delete;

    /**
     * Global access point.
     * Guaranteed thread-safe initialization.
     */
    static TelemetryRegistry& getInstance() {
        // TODO: Implement Meyers' Singleton or std::call_once initialization
        static TelemetryRegistry instance;
        return instance;
    }

    /**
     * Atomically increments the counter for a given metric.
     */
    void incrementCounter([[maybe_unused]] const std::string& metricName, [[maybe_unused]] uint64_t amount = 1) {
        // TODO: Safely increment counter under concurrent access
    }

    /**
     * Retrieves the current aggregated count for a given metric.
     */
    uint64_t getCounter([[maybe_unused]] const std::string& metricName) const {
        // TODO: Safely retrieve counter value
        return 0;
    }

    /**
     * Resets internal metrics (Used exclusively for test isolation).
     */
    void resetForTesting() {
        // TODO: Safely clear metrics map
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running Thread-Safe Telemetry Registry Test Suite <<<\n\n";

    // Test Case 1: Identity & Singleton Verification
    {
        std::cout << "[Test 1] Memory Address Uniqueness Verification... ";
        TelemetryRegistry& reg1 = TelemetryRegistry::getInstance();
        TelemetryRegistry& reg2 = TelemetryRegistry::getInstance();

        assert(&reg1 == &reg2 && "Both references must point to the exact same memory address");
        std::cout << "PASSED (Address: " << &reg1 << ")\n";
    }

    // Test Case 2: Single-threaded Counter Increment & Reset
    {
        std::cout << "[Test 2] Metric Recording & Isolation... ";
        auto& reg = TelemetryRegistry::getInstance();
        reg.resetForTesting();

        reg.incrementCounter("http_requests_total", 5);
        reg.incrementCounter("http_requests_total", 10);
        assert(reg.getCounter("http_requests_total") == 15 && "Total should equal 15");

        reg.resetForTesting();
        assert(reg.getCounter("http_requests_total") == 0 && "Counter must reset to 0");
        std::cout << "PASSED\n";
    }

    // Test Case 3: High-Concurrency Stress Test (16 threads x 1,000 increments)
    {
        std::cout << "[Test 3] Multi-threaded Concurrency Stress Test... ";
        auto& reg = TelemetryRegistry::getInstance();
        reg.resetForTesting();

        constexpr int numThreads = 16;
        constexpr int incrementsPerThread = 1000;
        std::vector<std::thread> workers;

        for (int i = 0; i < numThreads; ++i) {
            workers.emplace_back([&reg]() {
                for (int j = 0; j < incrementsPerThread; ++j) {
                    reg.incrementCounter("concurrent_events", 1);
                }
            });
        }

        for (auto& t : workers) {
            t.join();
        }

        constexpr uint64_t expected = numThreads * incrementsPerThread;
        uint64_t actual = reg.getCounter("concurrent_events");
        assert(actual == expected && "Data race detected: counter does not match expected total!");
        std::cout << "PASSED (" << actual << " increments without race conditions)\n";
    }

    std::cout << "\n🎉 ALL SINGLETON TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
