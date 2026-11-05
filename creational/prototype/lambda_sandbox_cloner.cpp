/**
 * ==============================================================================================
 * PROBLEM STATEMENT: AWS Lambda / Firecracker MicroVM Execution Sandbox Cloner (SnapStart)
 * Category: Creational Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * In AWS Lambda / Google Cloud Functions, spinning up an execution container from scratch
 * ("Cold Start") incurs a 500ms - 2s penalty to parse configs, load libraries, and warm runtimes.
 * 
 * To eliminate cold starts, modern serverless engines initialize a pre-warmed "Golden Sandbox"
 * once, then clone it in under 5ms ("SnapStart").
 * 
 * A shallow copy causes catastrophic bugs: two client executions would mutate the same
 * memory buffers, leak credentials across customer invocations, or share process IDs.
 * The system requires:
 *   1. Polymorphic deep copying of sandboxes via a unified `clone()` interface.
 *   2. Independent mutation: Modifying environment variables, PID, or memory state on a clone
 *      must NEVER leak into or alter the template prototype or sibling clones.
 *   3. A `SandboxRegistry` caching pre-warmed prototypes by runtime ID ("python3.11", "nodejs20").
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Deep Copy Semantics:
 *    - All nested structures (memory blocks, custom env variables) must be deeply cloned.
 * 2. Modern C++ Virtual Clone Idiom:
 *    - Implement `virtual std::unique_ptr<ExecutionSandbox> clone() const = 0;`.
 * 3. Covariant Returns / Unique Pointer Ownership:
 *    - Clean ownership transfer without memory leaks.
 * 4. Isolation Guarantee:
 *    - Mutating a clone must leave the prototype template intact.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you specify the kinds of objects to create using a prototypical instance,
 * and create new objects by copying this prototype without coupling to its concrete class?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Abstract Prototype Hierarchy (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class ExecutionSandbox {
public:
    virtual ~ExecutionSandbox() = default;

    // TODO: Define the prototype clone interface
    virtual std::unique_ptr<ExecutionSandbox> clone() const = 0;

    virtual void setEnvVar(const std::string& key, const std::string& value) = 0;
    virtual std::string getEnvVar(const std::string& key) const = 0;
    virtual std::string_view getRuntime() const noexcept = 0;
    virtual int getMemoryMb() const noexcept = 0;
    virtual void appendLog(const std::string& entry) = 0;
    virtual const std::vector<std::string>& getLogs() const noexcept = 0;
};

// ----------------------------------------------------------------------------------------------
// Concrete Prototype: Python ML Sandbox
// ----------------------------------------------------------------------------------------------

class PythonMLSandbox : public ExecutionSandbox {
private:
    std::string runtime_{"python3.11"};
    int memoryMb_{2048};
    std::unordered_map<std::string, std::string> envVars_;
    std::vector<std::string> logs_;

public:
    PythonMLSandbox() {
        envVars_["PYTHONPATH"] = "/var/runtime";
        envVars_["PREWARMED_MODEL"] = "resnet50_v2";
        logs_.push_back("Initialized base Python ML sandbox");
    }

    // Copy constructor for deep cloning
    PythonMLSandbox(const PythonMLSandbox& other)
        : runtime_(other.runtime_),
          memoryMb_(other.memoryMb_),
          envVars_(other.envVars_),
          logs_(other.logs_) {}

    std::unique_ptr<ExecutionSandbox> clone() const override {
        // TODO: Return a deep copy of this instance wrapped in unique_ptr
        return nullptr;
    }

    void setEnvVar(const std::string& key, const std::string& value) override {
        envVars_[key] = value;
    }

    std::string getEnvVar(const std::string& key) const override {
        auto it = envVars_.find(key);
        return it != envVars_.end() ? it->second : "";
    }

    std::string_view getRuntime() const noexcept override { return runtime_; }
    int getMemoryMb() const noexcept override { return memoryMb_; }

    void appendLog(const std::string& entry) override {
        logs_.push_back(entry);
    }

    const std::vector<std::string>& getLogs() const noexcept override {
        return logs_;
    }
};

/**
 * Prototype Registry:
 * Stores pre-warmed prototype instances and dispenses deep-cloned copies on demand.
 */
class SandboxPrototypeRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<ExecutionSandbox>> prototypes_;

public:
    void registerPrototype(const std::string& typeName, std::unique_ptr<ExecutionSandbox> prototype) {
        prototypes_[typeName] = std::move(prototype);
    }

    std::unique_ptr<ExecutionSandbox> createSandbox(const std::string& typeName) const {
        auto it = prototypes_.find(typeName);
        if (it != prototypes_.end() && it->second) {
            // TODO: Call prototype clone() to return a brand-new independent sandbox
            return nullptr;
        }
        return nullptr;
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running Lambda Sandbox Cloner (Prototype) Test Suite <<<\n\n";

    SandboxPrototypeRegistry registry;
    registry.registerPrototype("python-ml", std::make_unique<PythonMLSandbox>());

    // Test Case 1: Clone Creation & State Verification
    {
        std::cout << "[Test 1] Clone from Golden Prototype... ";
        auto sandbox = registry.createSandbox("python-ml");
        assert(sandbox != nullptr && "Cloned sandbox must not be null");
        assert(sandbox->getRuntime() == "python3.11");
        assert(sandbox->getMemoryMb() == 2048);
        assert(sandbox->getEnvVar("PREWARMED_MODEL") == "resnet50_v2");
        assert(sandbox->getLogs().size() == 1);
        std::cout << "PASSED\n";
    }

    // Test Case 2: Deep Isolation Verification
    // Modifying clone A must NEVER affect clone B or the underlying prototype!
    {
        std::cout << "[Test 2] Deep Isolation Verification Across Clones... ";
        auto cloneA = registry.createSandbox("python-ml");
        auto cloneB = registry.createSandbox("python-ml");

        assert(cloneA.get() != cloneB.get() && "Clones must have distinct memory addresses");

        // Mutate clone A
        cloneA->setEnvVar("CUSTOMER_AWS_ACCOUNT_ID", "111122223333");
        cloneA->appendLog("Processing customer A request");

        // Verify clone B remains pristine and untainted
        assert(cloneB->getEnvVar("CUSTOMER_AWS_ACCOUNT_ID").empty() && "Clone B must not see customer A's credentials!");
        assert(cloneB->getLogs().size() == 1 && "Clone B log history must not be polluted");

        // Mutate clone B
        cloneB->setEnvVar("CUSTOMER_AWS_ACCOUNT_ID", "999988887777");
        assert(cloneA->getEnvVar("CUSTOMER_AWS_ACCOUNT_ID") == "111122223333");
        assert(cloneB->getEnvVar("CUSTOMER_AWS_ACCOUNT_ID") == "999988887777");

        std::cout << "PASSED (Zero cross-tenant credential leakage)\n";
    }

    std::cout << "\n🎉 ALL PROTOTYPE TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
