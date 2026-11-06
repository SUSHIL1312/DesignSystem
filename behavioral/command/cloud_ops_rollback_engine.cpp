/*
 * Cloud Infrastructure Deployment & Rollback Engine
 *
 * Requirements:
 * - Encapsulate deployment actions (provisioning VMs, opening firewall ports) as command objects
 *   implementing ICloudCommand (execute and undo).
 * - Invoker (DeploymentPipeline) maintains an execution history stack.
 * - Support rolling back commands in reverse order if a deployment step fails.
 *
 * Practice Reference:
 * You can refer to https://systemdesignmastry.com/ for practicing interview-asked HLD & LLD questions.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Receiver Subsystems (Cloud Resources being manipulated)
// ----------------------------------------------------------------------------------------------

class CloudNetworkManager {
public:
    bool openPort(int port) {
        if (port == 9999) return false; // Simulated failure port
        std::cout << "  [Firewall] Opened port " << port << "\n";
        return true;
    }

    void closePort(int port) {
        std::cout << "  [Firewall] Closed port " << port << "\n";
    }
};

class CloudComputeManager {
public:
    bool createInstance(const std::string& vmId) {
        std::cout << "  [Compute] Created VM " << vmId << "\n";
        return true;
    }

    void deleteInstance(const std::string& vmId) {
        std::cout << "  [Compute] Deleted VM " << vmId << "\n";
    }
};

// ----------------------------------------------------------------------------------------------
// Command Interface & Concrete Commands (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class ICloudCommand {
public:
    virtual ~ICloudCommand() = default;

    virtual bool execute() = 0;
    virtual void undo() = 0;
    virtual std::string_view getName() const noexcept = 0;
};

class ProvisionVmCommand : public ICloudCommand {
private:
    CloudComputeManager& compute_;
    std::string vmId_;
    [[maybe_unused]] bool executed_{false};

public:
    ProvisionVmCommand(CloudComputeManager& compute, std::string vmId)
        : compute_(compute), vmId_(std::move(vmId)) {}

    bool execute() override {
        // TODO: Call compute_.createInstance(vmId_) and track executed_
        (void)compute_;
        return false;
    }

    void undo() override {
        // TODO: If executed_, call compute_.deleteInstance(vmId_)
    }

    std::string_view getName() const noexcept override { return "ProvisionVmCommand"; }
};

class OpenPortCommand : public ICloudCommand {
private:
    CloudNetworkManager& network_;
    int port_;
    [[maybe_unused]] bool executed_{false};

public:
    OpenPortCommand(CloudNetworkManager& network, int port)
        : network_(network), port_(port) {}

    bool execute() override {
        // TODO: Call network_.openPort(port_) and track executed_
        (void)network_; (void)port_;
        return false;
    }

    void undo() override {
        // TODO: If executed_, call network_.closePort(port_)
    }

    std::string_view getName() const noexcept override { return "OpenPortCommand"; }
};

// ----------------------------------------------------------------------------------------------
// Invoker: Transaction Pipeline (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class DeploymentPipeline {
private:
    std::vector<std::unique_ptr<ICloudCommand>> executedHistory_;

public:
    bool executeCommand(std::unique_ptr<ICloudCommand> command) {
        // TODO:
        // 1. Execute command
        // 2. If succeeds, push to executedHistory_ and return true
        // 3. If fails, return false
        (void)command;
        return false;
    }

    void rollbackAll() {
        // TODO: Iterate executedHistory_ in reverse, calling undo() on each, then clear history
    }

    size_t getHistoryCount() const noexcept {
        return executedHistory_.size();
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running cloud ops rollback engine tests...\n\n";

    CloudComputeManager compute;
    CloudNetworkManager network;
    DeploymentPipeline pipeline;

    // Test Case 1: Sequential Successful Execution
    {
        std::cout << "[Test 1] Successful Sequential Deployment... \n";
        bool ok1 = pipeline.executeCommand(std::make_unique<ProvisionVmCommand>(compute, "vm-prod-01"));
        bool ok2 = pipeline.executeCommand(std::make_unique<OpenPortCommand>(network, 443));

        assert(ok1 && ok2);
        assert(pipeline.getHistoryCount() == 2 && "Two commands should be logged in history");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Manual Rollback
    {
        std::cout << "[Test 2] Manual Rollback Execution... \n";
        pipeline.rollbackAll();
        assert(pipeline.getHistoryCount() == 0 && "History should be empty after complete rollback");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Automated Rollback on Step Failure
    {
        std::cout << "[Test 3] Automated Rollback on Mid-flight Step Failure... \n";
        bool step1 = pipeline.executeCommand(std::make_unique<ProvisionVmCommand>(compute, "vm-prod-02"));
        assert(step1);

        // Step 2 attempts invalid port 9999 -> fails!
        bool step2 = pipeline.executeCommand(std::make_unique<OpenPortCommand>(network, 9999));
        assert(!step2 && "Port 9999 must fail");

        // Roll back uncompleted transaction
        pipeline.rollbackAll();
        assert(pipeline.getHistoryCount() == 0);
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll cloud ops rollback engine tests passed.\n";
    return 0;
}
