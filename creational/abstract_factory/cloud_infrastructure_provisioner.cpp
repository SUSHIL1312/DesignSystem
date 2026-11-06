/*
 * Multi-Cloud Infrastructure Provisioner
 *
 * Requirements:
 * - Provision complete, compatible families of cloud resources:
 *   - Compute instance (EC2Instance / ComputeEngineInstance)
 *   - Storage volume (EBSVolume / PersistentDiskVolume)
 *   - Network gateway (VpcInternetGateway / CloudRouterGateway)
 * - Prevent mixing resources across cloud providers (e.g. attaching AWS storage to GCP compute).
 * - Allow orchestrator to deploy a full stack without depending on vendor-specific concrete classes.
 * - Support adding new cloud providers (e.g. Azure) without modifying existing code.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Abstract Product Hierarchy
// ----------------------------------------------------------------------------------------------

class IComputeInstance {
public:
    virtual ~IComputeInstance() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::string_view getProvider() const noexcept = 0;
    virtual std::string_view getInstanceType() const noexcept = 0;
};

class IStorageVolume {
public:
    virtual ~IStorageVolume() = default;
    virtual bool attachTo(const std::string& instanceId) = 0;
    virtual int getCapacityGb() const noexcept = 0;
    virtual std::string_view getProvider() const noexcept = 0;
};

class INetworkGateway {
public:
    virtual ~INetworkGateway() = default;
    virtual bool routePacket(const std::string& destIp, const std::string& packet) = 0;
    virtual std::string_view getProvider() const noexcept = 0;
};

// ----------------------------------------------------------------------------------------------
// Concrete Product Families (AWS Family)
// ----------------------------------------------------------------------------------------------

class EC2Instance : public IComputeInstance {
public:
    void start() override { /* TODO: Implement */ }
    void stop() override { /* TODO: Implement */ }
    std::string_view getProvider() const noexcept override { return "AWS"; }
    std::string_view getInstanceType() const noexcept override { return "m5.large"; }
};

class EBSVolume : public IStorageVolume {
private:
    [[maybe_unused]] int capacityGb_{100};
public:
    explicit EBSVolume(int capacityGb) : capacityGb_(capacityGb) {}
    bool attachTo([[maybe_unused]] const std::string& instanceId) override {
        // TODO: Implement
        return false;
    }
    int getCapacityGb() const noexcept override { return capacityGb_; }
    std::string_view getProvider() const noexcept override { return "AWS"; }
};

class VpcInternetGateway : public INetworkGateway {
public:
    bool routePacket([[maybe_unused]] const std::string& destIp, [[maybe_unused]] const std::string& packet) override {
        // TODO: Implement
        return false;
    }
    std::string_view getProvider() const noexcept override { return "AWS"; }
};

// ----------------------------------------------------------------------------------------------
// Concrete Product Families (GCP Family)
// ----------------------------------------------------------------------------------------------

class ComputeEngineInstance : public IComputeInstance {
public:
    void start() override { /* TODO: Implement */ }
    void stop() override { /* TODO: Implement */ }
    std::string_view getProvider() const noexcept override { return "GCP"; }
    std::string_view getInstanceType() const noexcept override { return "n2-standard-4"; }
};

class PersistentDiskVolume : public IStorageVolume {
private:
    [[maybe_unused]] int capacityGb_{200};
public:
    explicit PersistentDiskVolume(int capacityGb) : capacityGb_(capacityGb) {}
    bool attachTo([[maybe_unused]] const std::string& instanceId) override {
        // TODO: Implement
        return false;
    }
    int getCapacityGb() const noexcept override { return capacityGb_; }
    std::string_view getProvider() const noexcept override { return "GCP"; }
};

class CloudRouterGateway : public INetworkGateway {
public:
    bool routePacket([[maybe_unused]] const std::string& destIp, [[maybe_unused]] const std::string& packet) override {
        // TODO: Implement
        return false;
    }
    std::string_view getProvider() const noexcept override { return "GCP"; }
};

// ----------------------------------------------------------------------------------------------
// Abstract Factory Interface & Concrete Factories (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class ICloudResourceFactory {
public:
    virtual ~ICloudResourceFactory() = default;

    // TODO: Define factory methods to create the resource family
    virtual std::unique_ptr<IComputeInstance> createCompute() const = 0;
    virtual std::unique_ptr<IStorageVolume> createStorage(int capacityGb) const = 0;
    virtual std::unique_ptr<INetworkGateway> createNetwork() const = 0;
};

class AWSResourceFactory : public ICloudResourceFactory {
public:
    std::unique_ptr<IComputeInstance> createCompute() const override {
        // TODO: Implement
        return nullptr;
    }
    std::unique_ptr<IStorageVolume> createStorage(int capacityGb) const override {
        // TODO: Implement
        (void)capacityGb;
        return nullptr;
    }
    std::unique_ptr<INetworkGateway> createNetwork() const override {
        // TODO: Implement
        return nullptr;
    }
};

class GCPResourceFactory : public ICloudResourceFactory {
public:
    std::unique_ptr<IComputeInstance> createCompute() const override {
        // TODO: Implement
        return nullptr;
    }
    std::unique_ptr<IStorageVolume> createStorage(int capacityGb) const override {
        // TODO: Implement
        (void)capacityGb;
        return nullptr;
    }
    std::unique_ptr<INetworkGateway> createNetwork() const override {
        // TODO: Implement
        return nullptr;
    }
};

/**
 * Orchestrator / Client:
 * Manages an infrastructure stack without knowing concrete cloud vendor types.
 */
class CloudDeploymentManager {
private:
    std::unique_ptr<ICloudResourceFactory> factory_;

public:
    explicit CloudDeploymentManager(std::unique_ptr<ICloudResourceFactory> factory)
        : factory_(std::move(factory)) {}

    struct ProvisionedStack {
        std::unique_ptr<IComputeInstance> compute;
        std::unique_ptr<IStorageVolume> storage;
        std::unique_ptr<INetworkGateway> network;
    };

    ProvisionedStack deployStack(int diskSizeGb) {
        if (!factory_) return {};
        // TODO: Provision the stack using factory_ methods and ensure compatibility
        (void)diskSizeGb;
        return {};
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running multi-cloud infrastructure provisioner tests...\n\n";

    // Test Case 1: Deploy Full AWS Stack
    {
        std::cout << "[Test 1] Provision AWS Infrastructure Stack... ";
        CloudDeploymentManager awsDeployer(std::make_unique<AWSResourceFactory>());
        auto stack = awsDeployer.deployStack(500);

        assert(stack.compute != nullptr && "Compute instance must be created");
        assert(stack.storage != nullptr && "Storage volume must be created");
        assert(stack.network != nullptr && "Network gateway must be created");

        assert(stack.compute->getProvider() == "AWS");
        assert(stack.storage->getProvider() == "AWS");
        assert(stack.network->getProvider() == "AWS");
        assert(stack.storage->getCapacityGb() == 500);
        std::cout << "PASSED (AWS Stack verified)\n";
    }

    // Test Case 2: Deploy Full GCP Stack
    {
        std::cout << "[Test 2] Provision GCP Infrastructure Stack... ";
        CloudDeploymentManager gcpDeployer(std::make_unique<GCPResourceFactory>());
        auto stack = gcpDeployer.deployStack(1000);

        assert(stack.compute != nullptr);
        assert(stack.storage != nullptr);
        assert(stack.network != nullptr);

        assert(stack.compute->getProvider() == "GCP");
        assert(stack.storage->getProvider() == "GCP");
        assert(stack.network->getProvider() == "GCP");
        assert(stack.storage->getCapacityGb() == 1000);
        std::cout << "PASSED (GCP Stack verified)\n";
    }

    // Test Case 3: Dynamic 3rd Party Azure Extension (OCP Validation)
    {
        std::cout << "[Test 3] Dynamic Extension - Third Party Azure Cloud Factory... ";

        class AzureVM : public IComputeInstance {
        public:
            void start() override {}
            void stop() override {}
            std::string_view getProvider() const noexcept override { return "Azure"; }
            std::string_view getInstanceType() const noexcept override { return "Standard_D4s_v5"; }
        };

        class AzureDisk : public IStorageVolume {
            int cap_;
        public:
            explicit AzureDisk(int cap) : cap_(cap) {}
            bool attachTo(const std::string&) override { return true; }
            int getCapacityGb() const noexcept override { return cap_; }
            std::string_view getProvider() const noexcept override { return "Azure"; }
        };

        class AzureVNetGateway : public INetworkGateway {
        public:
            bool routePacket(const std::string&, const std::string&) override { return true; }
            std::string_view getProvider() const noexcept override { return "Azure"; }
        };

        class AzureResourceFactory : public ICloudResourceFactory {
        public:
            std::unique_ptr<IComputeInstance> createCompute() const override {
                return std::make_unique<AzureVM>();
            }
            std::unique_ptr<IStorageVolume> createStorage(int capacityGb) const override {
                return std::make_unique<AzureDisk>(capacityGb);
            }
            std::unique_ptr<INetworkGateway> createNetwork() const override {
                return std::make_unique<AzureVNetGateway>();
            }
        };

        CloudDeploymentManager azureDeployer(std::make_unique<AzureResourceFactory>());
        auto stack = azureDeployer.deployStack(250);

        assert(stack.compute && stack.storage && stack.network);
        assert(stack.compute->getProvider() == "Azure");
        assert(stack.storage->getProvider() == "Azure");
        assert(stack.network->getProvider() == "Azure");
        std::cout << "PASSED (Azure extension deployed without touching core code)\n";
    }

    std::cout << "\nAll multi-cloud infrastructure provisioner tests passed.\n";
    return 0;
}
