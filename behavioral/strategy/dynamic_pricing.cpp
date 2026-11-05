/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Uber / Amazon Dynamic Surge Pricing & Delivery Cost Optimizer
 * Category: Behavioral Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * At Uber / Amazon Logistics, delivery and ride pricing is dynamically evaluated based on
 * live traffic, weather, supply-demand imbalance, and user subscription tiers.
 * 
 * Business Requirements:
 *   1. Standard Pricing Strategy:
 *      - Base fare: $3.00
 *      - Per km: $1.50
 *      - Per minute: $0.25
 *   2. Surge Pricing Strategy (Peak Rush / Heavy Rain):
 *      - Standard fare multiplied by surge factor (e.g., 1.5x, 2.0x).
 *      - Regulatory safety constraint: Surge factor capped at 2.5x max, floored at 1.0x.
 *   3. Prime Loyalty / VIP Strategy:
 *      - Base fare waived ($0.00).
 *      - Discounted distance rate: $1.10 per km.
 *      - Normal time rate: $0.25 per minute.
 *      - If distance <= 5 km, time charge is also waived entirely.
 * 
 * The pricing engine in the Checkout microservice must allow algorithms to be selected,
 * swapped, or configured at runtime (e.g., a sudden downpour triggers surge pricing; a user applies
 * a Prime membership at checkout). Hardcoding `if-else` or `switch (pricingMode)` inside the
 * order service violates the Open-Closed Principle and fails L5/L6 bar.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Open-Closed Principle (OCP):
 *    - New pricing algorithms (e.g., Black Friday Flash Sale, EV Green Fleet discount) must be
 *      introducible without modifying existing strategies or the context engine.
 * 2. Dynamic Runtime Swapping:
 *    - The pricing context must support hot-swapping its active calculation strategy at runtime.
 * 3. Modern C++ Design:
 *    - Ownership managed via `std::unique_ptr<IPricingStrategy>`.
 *    - Const-correctness on inspection methods (`calculateFare(const RideDetails&)`).
 * 4. Resilient Validation:
 *    - Invalid ride parameters (negative distance, negative duration) must throw `std::invalid_argument`.
 * 5. Deterministic Floating Point / Currency Handling:
 *    - Fares rounded to 2 decimal places or calculated with high precision doubles.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you encapsulate a family of algorithms behind a polymorphic interface such that
 * the algorithm can vary completely independently from the client using it?
 * What mechanism allows you to change the behavior of the calculation context at runtime?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cmath>
#include <cassert>
#include <stdexcept>
#include <iomanip>

// ----------------------------------------------------------------------------------------------
// Domain Models
// ----------------------------------------------------------------------------------------------

struct RideDetails {
    double distanceKm{0.0};
    double durationMinutes{0.0};
    double packageWeightKg{0.0};
    bool isPeakHour{false};
};

struct FareBreakdown {
    double totalFare{0.0};
    std::string strategyName;
    std::string calculationDetails;
};

// ----------------------------------------------------------------------------------------------
// Skeletons & Signatures (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

/**
 * Strategy Interface: Contract for all pricing calculation algorithms.
 */
class IPricingStrategy {
public:
    virtual ~IPricingStrategy() = default;

    // TODO: Define the calculation contract
    virtual FareBreakdown calculateFare(const RideDetails& ride) const = 0;
    virtual std::string_view getStrategyName() const noexcept = 0;
};

/**
 * Concrete Strategy 1: Standard Distance & Time Pricing
 * Formula:
 *   Base: $3.00
 *   Distance: $1.50 / km
 *   Time: $0.25 / min
 */
class StandardPricingStrategy : public IPricingStrategy {
public:
    FareBreakdown calculateFare(const RideDetails& ride) const override {
        // TODO: Validate ride and compute fare
        (void)ride;
        return FareBreakdown{.totalFare = 0.0, .strategyName = "Standard"};
    }

    std::string_view getStrategyName() const noexcept override {
        return "Standard";
    }
};

/**
 * Concrete Strategy 2: Surge Pricing
 * Formula:
 *   Standard Fare * surgeMultiplier
 *   Constraints:
 *   - surgeMultiplier must be clamped between [1.0, 2.5]
 */
class SurgePricingStrategy : public IPricingStrategy {
private:
    [[maybe_unused]] double multiplier_;

public:
    explicit SurgePricingStrategy(double multiplier) : multiplier_(multiplier) {
        // TODO: Enforce [1.0, 2.5] bounds
    }

    FareBreakdown calculateFare(const RideDetails& ride) const override {
        // TODO: Compute standard fare and apply clamped surge multiplier
        (void)ride;
        return FareBreakdown{.totalFare = 0.0, .strategyName = "Surge"};
    }

    std::string_view getStrategyName() const noexcept override {
        return "Surge";
    }
};

/**
 * Concrete Strategy 3: Prime Loyalty / VIP Pricing
 * Formula:
 *   Base: $0.00 (waived)
 *   Distance: $1.10 / km
 *   Time: $0.25 / min (waived if distanceKm <= 5.0 km)
 */
class PrimeLoyaltyPricingStrategy : public IPricingStrategy {
public:
    FareBreakdown calculateFare(const RideDetails& ride) const override {
        // TODO: Implement Prime subscription pricing logic
        (void)ride;
        return FareBreakdown{.totalFare = 0.0, .strategyName = "PrimeLoyalty"};
    }

    std::string_view getStrategyName() const noexcept override {
        return "PrimeLoyalty";
    }
};

/**
 * Context: Manages the active pricing strategy and exposes fare calculation to the checkout pipeline.
 */
class FareCalculationContext {
private:
    std::unique_ptr<IPricingStrategy> strategy_;

public:
    explicit FareCalculationContext(std::unique_ptr<IPricingStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    // Allows hot-swapping the strategy at runtime
    void setStrategy(std::unique_ptr<IPricingStrategy> newStrategy) {
        strategy_ = std::move(newStrategy);
    }

    FareBreakdown calculate(const RideDetails& ride) const {
        if (!strategy_) {
            throw std::runtime_error("No pricing strategy configured");
        }
        return strategy_->calculateFare(ride);
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running Dynamic Surge Pricing Test Suite <<<\n\n";

    auto roundTwoDecimals = [](double val) {
        return std::round(val * 100.0) / 100.0;
    };

    // Test Case 1: Standard Pricing Strategy
    // 10 km, 20 mins -> Base ($3.00) + 10 * 1.50 ($15.00) + 20 * 0.25 ($5.00) = $23.00
    {
        std::cout << "[Test 1] Standard Pricing Calculation... ";
        FareCalculationContext context(std::make_unique<StandardPricingStrategy>());

        RideDetails ride{.distanceKm = 10.0, .durationMinutes = 20.0};
        FareBreakdown fare = context.calculate(ride);

        assert(roundTwoDecimals(fare.totalFare) == 23.00 && "Standard fare for 10km/20min should be $23.00");
        assert(fare.strategyName == "Standard");
        std::cout << "PASSED ($" << fare.totalFare << ")\n";
    }

    // Test Case 2: Validation of Invalid Inputs
    {
        std::cout << "[Test 2] Invalid Input Validation (Negative distance/duration)... ";
        FareCalculationContext context(std::make_unique<StandardPricingStrategy>());

        bool caught = false;
        try {
            RideDetails invalidRide{.distanceKm = -5.0, .durationMinutes = 10.0};
            context.calculate(invalidRide);
        } catch (const std::invalid_argument&) {
            caught = true;
        }
        assert(caught && "Negative distance must throw std::invalid_argument");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Surge Pricing Strategy with Multiplier
    // 10 km, 20 mins -> Standard is $23.00. Surge 1.5x -> $34.50
    {
        std::cout << "[Test 3] Surge Pricing (1.5x multiplier)... ";
        FareCalculationContext context(std::make_unique<SurgePricingStrategy>(1.5));

        RideDetails ride{.distanceKm = 10.0, .durationMinutes = 20.0};
        FareBreakdown fare = context.calculate(ride);

        assert(roundTwoDecimals(fare.totalFare) == 34.50 && "Surge fare should be $34.50");
        assert(fare.strategyName == "Surge");
        std::cout << "PASSED ($" << fare.totalFare << ")\n";
    }

    // Test Case 4: Surge Multiplier Clamping to Regulatory Cap (Max 2.5x)
    // Attempting 4.0x should clamp to 2.5x. $23.00 * 2.5 = $57.50
    {
        std::cout << "[Test 4] Surge Cap Clamping (4.0x clamped to 2.5x max)... ";
        FareCalculationContext context(std::make_unique<SurgePricingStrategy>(4.0));

        RideDetails ride{.distanceKm = 10.0, .durationMinutes = 20.0};
        FareBreakdown fare = context.calculate(ride);

        assert(roundTwoDecimals(fare.totalFare) == 57.50 && "Clamped surge fare should be $57.50");
        std::cout << "PASSED ($" << fare.totalFare << ")\n";
    }

    // Test Case 5: Prime Loyalty Strategy
    // 5a: <= 5km (Time fee waived): 4 km, 15 mins -> 4 * $1.10 = $4.40
    // 5b: > 5km (Time fee applies): 10 km, 20 mins -> 10 * $1.10 ($11.00) + 20 * 0.25 ($5.00) = $16.00
    {
        std::cout << "[Test 5] Prime Loyalty Subscription Pricing... ";
        FareCalculationContext context(std::make_unique<PrimeLoyaltyPricingStrategy>());

        RideDetails shortRide{.distanceKm = 4.0, .durationMinutes = 15.0};
        FareBreakdown shortFare = context.calculate(shortRide);
        assert(roundTwoDecimals(shortFare.totalFare) == 4.40 && "Short Prime ride should waive time fee");

        RideDetails longRide{.distanceKm = 10.0, .durationMinutes = 20.0};
        FareBreakdown longFare = context.calculate(longRide);
        assert(roundTwoDecimals(longFare.totalFare) == 16.00 && "Long Prime ride should charge discounted rate");

        std::cout << "PASSED (Short: $" << shortFare.totalFare << ", Long: $" << longFare.totalFare << ")\n";
    }

    // Test Case 6: Dynamic Runtime Strategy Swapping
    // Scenario: User starts ride with Standard pricing, but during booking applies Prime Membership!
    {
        std::cout << "[Test 6] Dynamic Runtime Strategy Swapping on Active Context... ";
        FareCalculationContext context(std::make_unique<StandardPricingStrategy>());

        RideDetails ride{.distanceKm = 10.0, .durationMinutes = 20.0};
        assert(roundTwoDecimals(context.calculate(ride).totalFare) == 23.00);

        // User applies Prime membership -> Hot-swap strategy
        context.setStrategy(std::make_unique<PrimeLoyaltyPricingStrategy>());
        assert(roundTwoDecimals(context.calculate(ride).totalFare) == 16.00);

        // Heavy thunderstorm begins -> Dispatcher updates to Surge
        context.setStrategy(std::make_unique<SurgePricingStrategy>(2.0));
        assert(roundTwoDecimals(context.calculate(ride).totalFare) == 46.00);

        std::cout << "PASSED\n";
    }

    // Test Case 7: OCP Verification - Dynamic 3rd Party Holiday Flash Strategy
    {
        std::cout << "[Test 7] OCP Verification - Custom Holiday Flat Promo Strategy... ";

        class HolidayPromoStrategy : public IPricingStrategy {
        public:
            FareBreakdown calculateFare(const RideDetails& ride) const override {
                if (ride.distanceKm < 0 || ride.durationMinutes < 0) {
                    throw std::invalid_argument("Negative values not permitted");
                }
                // Flat $10 holiday ride regardless of distance
                return FareBreakdown{
                    .totalFare = 10.00,
                    .strategyName = "HolidayPromo",
                    .calculationDetails = "Flat $10.00 Holiday special"
                };
            }
            std::string_view getStrategyName() const noexcept override {
                return "HolidayPromo";
            }
        };

        FareCalculationContext context(std::make_unique<HolidayPromoStrategy>());
        RideDetails ride{.distanceKm = 35.0, .durationMinutes = 75.0};
        FareBreakdown fare = context.calculate(ride);
        assert(roundTwoDecimals(fare.totalFare) == 10.00);
        assert(fare.strategyName == "HolidayPromo");
        std::cout << "PASSED\n";
    }

    std::cout << "\n🎉 ALL DYNAMIC PRICING STRATEGY TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
