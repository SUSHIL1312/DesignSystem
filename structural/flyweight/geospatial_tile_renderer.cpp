/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Google Maps Geospatial Landmark & POI Render Cache
 * Category: Structural Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * In Google Maps 3D view, a metropolitan area (e.g., Manhattan or Tokyo) displays hundreds of
 * thousands of landmarks, street furniture, and POIs (trees, traffic lights, EV chargers, subway pins).
 * 
 * Every landmark has:
 *   1. Intrinsic State (Invariant & Shared):
 *      - 3D Model Mesh Data, 4K Texture Atlas, POI Category, Icon Glyph.
 *      - Memory footprint: ~2 MB per unique landmark archetype.
 *   2. Extrinsic State (Dynamic & Per-Instance):
 *      - Exact GPS coordinates (latitude, longitude, altitude), orientation angle, label text.
 * 
 * If the client instantiates 50,000 trees eagerly, allocating duplicate mesh and texture data
 * consumes $50,000 \times 2\text{ MB} = 100\text{ GB}$ of RAM, crashing client devices immediately.
 * 
 * The system must share invariant intrinsic state across thousands of map markers, drastically
 * reducing memory consumption while keeping extrinsic positions independent.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Intrinsic / Extrinsic Separation:
 *    - `LandmarkType` flyweight stores heavy invariant assets (texture, mesh, category).
 *    - `PlacedMapMarker` stores extrinsic spatial state (lat, lon, label).
 * 2. Flyweight Factory with Lazy Caching:
 *    - `LandmarkTypeFactory` caches shared flyweights by key ("OakTree", "TrafficSignal").
 *    - If requested type already exists, return the cached instance; do NOT reallocate.
 * 3. Modern C++ Shared Semantics:
 *    - Manage shared flyweights safely via `std::shared_ptr<const LandmarkType>`.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you use sharing to support large numbers of fine-grained objects efficiently?
 * What data belongs inside the shared object, and what data must be supplied from the outside?
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
// Flyweight: Invariant Intrinsic State (Heavy 3D Mesh / Texture Data)
// ----------------------------------------------------------------------------------------------

class LandmarkType {
private:
    std::string category_;      // "Tree", "EV_Charger", "Subway_Pin"
    std::string textureData_;   // Simulates large 2MB texture blob
    std::string meshBlob_;      // Simulates 3D polygon geometry

public:
    LandmarkType(std::string category, std::string textureData, std::string meshBlob)
        : category_(std::move(category)),
          textureData_(std::move(textureData)),
          meshBlob_(std::move(meshBlob)) {
        std::cout << "  [Asset Allocation] Created Heavy LandmarkType archetype: " << category_ << "\n";
    }

    std::string_view getCategory() const noexcept { return category_; }
    size_t getAssetByteSize() const noexcept { return textureData_.size() + meshBlob_.size(); }
};

// ----------------------------------------------------------------------------------------------
// Flyweight Factory: Caches & Dispenses Shared Flyweights (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class LandmarkTypeFactory {
private:
    std::unordered_map<std::string, std::shared_ptr<LandmarkType>> typeCache_;

public:
    std::shared_ptr<LandmarkType> getLandmarkType(const std::string& category) {
        // TODO:
        // 1. Look up category in typeCache_
        // 2. If present, return cached shared_ptr
        // 3. If absent, allocate new LandmarkType, store in cache, and return
        (void)category;
        return nullptr;
    }

    size_t getUniqueTypesCount() const noexcept {
        return typeCache_.size();
    }
};

// ----------------------------------------------------------------------------------------------
// Context: Extrinsic State (Unique per placed marker)
// ----------------------------------------------------------------------------------------------

class PlacedMapMarker {
private:
    double latitude_{0.0};
    double longitude_{0.0};
    std::string label_;
    std::shared_ptr<const LandmarkType> type_; // Pointer to shared Flyweight

public:
    PlacedMapMarker(double lat, double lon, std::string label, std::shared_ptr<const LandmarkType> type)
        : latitude_(lat), longitude_(lon), label_(std::move(label)), type_(std::move(type)) {}

    void render() const {
        if (!type_) return;
        // Extrinsic state combined with shared intrinsic state
        // std::cout << "Rendering " << type_->getCategory() << " at (" << latitude_ << ", " << longitude_ << ")\n";
    }

    double getLatitude() const noexcept { return latitude_; }
    double getLongitude() const noexcept { return longitude_; }
    std::string_view getLabel() const noexcept { return label_; }
    std::shared_ptr<const LandmarkType> getType() const noexcept { return type_; }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running Google Maps Landmark Renderer (Flyweight) Test Suite <<<\n\n";

    LandmarkTypeFactory factory;

    // Test Case 1: Shared Flyweight Allocation
    {
        std::cout << "[Test 1] Flyweight Re-use Verification... ";
        auto treeType1 = factory.getLandmarkType("OakTree");
        auto treeType2 = factory.getLandmarkType("OakTree");

        assert(treeType1 != nullptr && "Factory should return valid flyweight");
        assert(treeType1.get() == treeType2.get() && "Both requests must return the exact same flyweight memory address!");
        assert(factory.getUniqueTypesCount() == 1 && "Only 1 unique archetype must exist in factory cache");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Large-Scale Memory Efficiency Simulation (10,000 Markers)
    {
        std::cout << "[Test 2] Scaled Marker Generation (10,000 markers sharing 3 flyweights)... ";
        std::vector<PlacedMapMarker> mapMarkers;
        mapMarkers.reserve(10000);

        for (int i = 0; i < 5000; ++i) {
            auto treeType = factory.getLandmarkType("OakTree");
            mapMarkers.emplace_back(37.7749 + (i * 0.0001), -122.4194 + (i * 0.0001), "Park Tree #" + std::to_string(i), treeType);
        }

        for (int i = 0; i < 3000; ++i) {
            auto signalType = factory.getLandmarkType("TrafficSignal");
            mapMarkers.emplace_back(37.7750 + (i * 0.0001), -122.4180 + (i * 0.0001), "Signal #" + std::to_string(i), signalType);
        }

        for (int i = 0; i < 2000; ++i) {
            auto chargerType = factory.getLandmarkType("EVCharger");
            mapMarkers.emplace_back(37.7760 + (i * 0.0001), -122.4150 + (i * 0.0001), "EV Station #" + std::to_string(i), chargerType);
        }

        assert(mapMarkers.size() == 10000);
        assert(factory.getUniqueTypesCount() == 3 && "Only 3 heavy landmark objects should exist in memory for 10,000 markers!");

        // Verify independent extrinsic locations
        assert(mapMarkers[0].getLatitude() != mapMarkers[1].getLatitude());
        // Verify shared intrinsic pointer
        assert(mapMarkers[0].getType().get() == mapMarkers[1].getType().get());

        std::cout << "PASSED (10,000 markers cleanly sharing 3 archetypes)\n";
    }

    std::cout << "\n🎉 ALL FLYWEIGHT TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
