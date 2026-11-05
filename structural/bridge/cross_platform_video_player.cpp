/**
 * ==============================================================================================
 * PROBLEM STATEMENT: YouTube Cross-Platform Video Player Engine (Decoupling UI & Graphics)
 * Category: Structural Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * The YouTube client runs across Smart TVs, Android, iOS, Windows, and macOS.
 * The architecture faces two independent axes of change:
 *   1. Feature Abstraction Axis:
 *      - `StandardPlayer`: Handles play/pause, buffer management, 1080p stream switching.
 *      - `AccessibilityPlayer`: Adds text-to-speech description and closed caption overlay rendering.
 *   2. Platform Hardware Rendering Axis:
 *      - Apple (iOS/macOS): Apple Metal API (`MetalRendererBackend`)
 *      - Windows: Microsoft DirectX 12 (`DirectXRendererBackend`)
 *      - Android/Linux: Khronos Vulkan (`VulkanRendererBackend`)
 * 
 * If combined through classic subclassing:
 *   `MetalStandardPlayer`, `DirectXStandardPlayer`, `MetalAccessibilityPlayer`, `DirectXAccessibilityPlayer`...
 * Every new player capability multiplied by every platform backend yields an $M \times N$ class explosion.
 * 
 * The system must bridge the high-level player abstraction with the low-level rendering backend,
 * letting both hierarchies vary independently.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Orthogonal Extension:
 *    - New player abstractions (e.g. `LiveStreamInteractivePlayer`) can be added without modifying backends.
 *    - New rendering backends (e.g. `WebGLRenderer`) can be added without modifying player abstractions.
 * 2. Bridge Composition:
 *    - The abstraction holds a `std::unique_ptr<IRendererBackend>` bridge reference.
 * 3. Modern C++:
 *    - Clean ownership, pure virtual contracts, no raw pointers.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you decouple an abstraction from its implementation so that the two can vary independently?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Implementor Hierarchy (Low-level rendering backends)
// ----------------------------------------------------------------------------------------------

class IRendererBackend {
public:
    virtual ~IRendererBackend() = default;

    virtual void drawVideoFrame(int frameId) = 0;
    virtual void renderOverlayText(const std::string& text) = 0;
    virtual std::string_view getBackendName() const noexcept = 0;
};

class MetalRendererBackend : public IRendererBackend {
public:
    void drawVideoFrame([[maybe_unused]] int frameId) override {
        // TODO: Metal draw calls
    }
    void renderOverlayText([[maybe_unused]] const std::string& text) override {
        // TODO: Metal overlay pipeline
    }
    std::string_view getBackendName() const noexcept override { return "Metal"; }
};

class VulkanRendererBackend : public IRendererBackend {
public:
    void drawVideoFrame([[maybe_unused]] int frameId) override {
        // TODO: Vulkan draw calls
    }
    void renderOverlayText([[maybe_unused]] const std::string& text) override {
        // TODO: Vulkan overlay pipeline
    }
    std::string_view getBackendName() const noexcept override { return "Vulkan"; }
};

// ----------------------------------------------------------------------------------------------
// Abstraction Hierarchy (High-level video player interfaces) (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class VideoPlayer {
protected:
    std::unique_ptr<IRendererBackend> backend_;

public:
    explicit VideoPlayer(std::unique_ptr<IRendererBackend> backend)
        : backend_(std::move(backend)) {}

    virtual ~VideoPlayer() = default;

    // TODO: Define high-level player contract
    virtual void playStream(const std::string& videoUrl) = 0;
    virtual std::string_view getActiveBackend() const noexcept {
        return backend_ ? backend_->getBackendName() : "None";
    }
};

class StandardVideoPlayer : public VideoPlayer {
public:
    explicit StandardVideoPlayer(std::unique_ptr<IRendererBackend> backend)
        : VideoPlayer(std::move(backend)) {}

    void playStream([[maybe_unused]] const std::string& videoUrl) override {
        // TODO: Render stream frames via backend_->drawVideoFrame(...)
    }
};

class AccessibilityVideoPlayer : public VideoPlayer {
public:
    explicit AccessibilityVideoPlayer(std::unique_ptr<IRendererBackend> backend)
        : VideoPlayer(std::move(backend)) {}

    void playStream([[maybe_unused]] const std::string& videoUrl) override {
        // TODO: Render frames AND display closed captions via backend_->renderOverlayText(...)
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running YouTube Player (Bridge) Test Suite <<<\n\n";

    // Test Case 1: Standard Player with Metal backend (iOS)
    {
        std::cout << "[Test 1] Standard Player on Apple Metal... ";
        auto player = std::make_unique<StandardVideoPlayer>(
            std::make_unique<MetalRendererBackend>()
        );
        assert(player->getActiveBackend() == "Metal");
        player->playStream("https://youtube.com/watch?v=dQw4w9WgXcQ");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Accessibility Player with Vulkan backend (Android)
    {
        std::cout << "[Test 2] Accessibility Player on Vulkan... ";
        auto player = std::make_unique<AccessibilityVideoPlayer>(
            std::make_unique<VulkanRendererBackend>()
        );
        assert(player->getActiveBackend() == "Vulkan");
        player->playStream("https://youtube.com/watch?v=sample");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Dynamic 3rd Party Backend Extension (DirectX on Windows) without touching Player
    {
        std::cout << "[Test 3] Dynamic Extension - DirectX12 Backend... ";

        class DirectX12Backend : public IRendererBackend {
        public:
            void drawVideoFrame(int) override {}
            void renderOverlayText(const std::string&) override {}
            std::string_view getBackendName() const noexcept override { return "DirectX12"; }
        };

        auto winPlayer = std::make_unique<StandardVideoPlayer>(
            std::make_unique<DirectX12Backend>()
        );
        assert(winPlayer->getActiveBackend() == "DirectX12");
        std::cout << "PASSED\n";
    }

    std::cout << "\n🎉 ALL BRIDGE TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
