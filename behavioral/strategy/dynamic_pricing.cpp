/*
 * Dynamic Fare Calculation & Pricing Engine
 *
 * Requirements:
 * - Encapsulate pricing algorithms behind an IPricingStrategy interface.
 * - Concrete strategies:
 *   - Standard: base fare + distance rate + time rate.
 *   - Surge: standard fare multiplied by surge factor (clamped between 1.0 and 2.5).
 *   - Loyalty/Prime: waived base fare, discounted distance, waived time for <= 5 km.
 * - Support runtime hot-swapping of pricing strategies on the calculation context.
 * - Validate ride parameters (throw std::invalid_argument on negative values).
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
    std::cout << "Running dynamic pricing strategy tests...\n\n";

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

    std::cout << "\nAll dynamic pricing strategy tests passed.\n";
    return 0;
}
