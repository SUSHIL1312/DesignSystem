/*
 * Document Asset Virtual & Protection Proxy
 *
 * Requirements:
 * - Uniform IDocumentAsset interface for both heavy real assets and proxies.
 * - Virtual proxy: defer expensive disk/network loading of heavy asset until first getContent() call.
 * - Protection proxy: verify user permissions and throw std::runtime_error on unauthorized access.
 * - Caching proxy: avoid redundant disk loads on subsequent reads.
 *
 * Practice Reference:
 * You can refer to https://systemdesignmastry.com/ for practicing interview-asked HLD & LLD questions.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cassert>
#include <stdexcept>

// ----------------------------------------------------------------------------------------------
// Subject Interface (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class IDocumentAsset {
public:
    virtual ~IDocumentAsset() = default;
    virtual std::string_view getContent() = 0;
    virtual size_t getByteSize() const = 0;
};

// ----------------------------------------------------------------------------------------------
// Real Subject: Expensive Heavy Asset
// ----------------------------------------------------------------------------------------------

class HeavyDocumentAsset : public IDocumentAsset {
private:
    std::string assetId_;
    std::string loadedData_;
    static inline int diskLoadCount_{0};

public:
    explicit HeavyDocumentAsset(std::string assetId) : assetId_(std::move(assetId)) {
        // Expensive disk / network simulation
        ++diskLoadCount_;
        std::cout << "  [Disk I/O] Loading 50MB heavy asset '" << assetId_ << "' from remote storage...\n";
        loadedData_ = "BLOB_DATA_FOR_" + assetId_;
    }

    std::string_view getContent() override {
        return loadedData_;
    }

    size_t getByteSize() const override {
        return loadedData_.size();
    }

    static int getDiskLoadCount() noexcept {
        return diskLoadCount_;
    }

    static void resetDiskLoadCount() noexcept {
        diskLoadCount_ = 0;
    }
};

// ----------------------------------------------------------------------------------------------
// Proxy Implementation (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

enum class UserRole {
    GUEST,
    COLLABORATOR,
    ADMIN
};

class SecureDocumentCacheProxy : public IDocumentAsset {
private:
    std::string assetId_;
    [[maybe_unused]] UserRole userRole_;
    std::unique_ptr<HeavyDocumentAsset> realAsset_;

public:
    SecureDocumentCacheProxy(std::string assetId, UserRole userRole)
        : assetId_(std::move(assetId)), userRole_(userRole) {}

    std::string_view getContent() override {
        // TODO:
        // 1. Protection check: If userRole_ == GUEST, throw std::runtime_error("Access Denied")
        // 2. Virtual lazy-load: If !realAsset_, instantiate HeavyDocumentAsset(assetId_)
        // 3. Delegate to realAsset_->getContent()
        return "";
    }

    size_t getByteSize() const override {
        // TODO: Return size if loaded, or 0 if not yet loaded
        return realAsset_ ? realAsset_->getByteSize() : 0;
    }

    bool isLoaded() const noexcept {
        return realAsset_ != nullptr;
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running secure document proxy tests...\n\n";

    HeavyDocumentAsset::resetDiskLoadCount();

    // Test Case 1: Lazy Virtual Loading Verification
    {
        std::cout << "[Test 1] Lazy Initialization (No eager disk I/O)... ";
        SecureDocumentCacheProxy proxy("arch_diagram_v3.png", UserRole::COLLABORATOR);

        // Object created, but should NOT have triggered heavy disk load yet!
        assert(!proxy.isLoaded() && "Real asset must NOT be eagerly loaded upon proxy creation");
        assert(HeavyDocumentAsset::getDiskLoadCount() == 0 && "Disk load count must be 0 initially");
        std::cout << "PASSED\n";

        // Access content -> triggers lazy load
        std::cout << "[Test 1b] On-demand loading on first read... ";
        std::string_view content = proxy.getContent();
        assert(proxy.isLoaded() && "Asset should now be loaded");
        assert(content == "BLOB_DATA_FOR_arch_diagram_v3.png");
        assert(HeavyDocumentAsset::getDiskLoadCount() == 1);
        std::cout << "PASSED\n";

        // Subsequent access -> verifies caching (no extra disk load)
        std::cout << "[Test 1c] Cache hit (No duplicate I/O)... ";
        (void)proxy.getContent();
        assert(HeavyDocumentAsset::getDiskLoadCount() == 1 && "Subsequent reads must use cached asset");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Protection Proxy (Access Control Enforcement)
    {
        std::cout << "[Test 2] Protection Proxy RBAC Access Rejection... ";
        SecureDocumentCacheProxy unauthorizedProxy("confidential_financials.pdf", UserRole::GUEST);

        bool accessBlocked = false;
        try {
            unauthorizedProxy.getContent();
        } catch (const std::runtime_error&) {
            accessBlocked = true;
        }

        assert(accessBlocked && "Guest role must be blocked from reading confidential asset");
        assert(!unauthorizedProxy.isLoaded() && "Asset must never be fetched into memory on access denial");
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll secure document proxy tests passed.\n";
    return 0;
}
