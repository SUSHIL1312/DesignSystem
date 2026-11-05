/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Amazon Prime 1-Click Buy Orchestrator (Subsystem Simplification)
 * Category: Structural Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * When an Amazon Prime customer taps "Buy Now with 1-Click", they expect an instantaneous,
 * frictionless purchase. Behind the scenes, five independent backend microservices must interact:
 *   1. `InventoryService`: Validates stock, locks item, and decrements warehouse inventory.
 *   2. `TaxCalculationService`: Evaluates regional sales tax based on customer delivery address.
 *   3. `PaymentBillingService`: Charges the customer's default 1-Click credit card.
 *   4. `ShippingLogisticsService`: Creates warehouse pick-list and assigns courier tracking ID.
 *   5. `NotificationService`: Dispatches push/email confirmation with delivery tracking.
 * 
 * Forcing mobile apps or web frontends to coordinate these 5 subsystems creates tight coupling,
 * security vulnerabilities, high network latency over cellular connections, and broken state
 * if a network packet drops halfway through.
 * 
 * The system requires a clean, high-level Facade that masks subsystem complexity behind
 * a single unified call: `checkout(customerId, itemId, quantity)`.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Unified Simple Interface:
 *    - Mask subsystem initialization, coordination sequence, and rollbacks behind `OneClickCheckoutFacade`.
 * 2. Loose Coupling:
 *    - Subsystems remain independent and can still be used directly by advanced fulfillment workflows.
 * 3. Transactional Fallback / Short-Circuiting:
 *    - If inventory is unavailable or payment fails, abort downstream fulfillment steps and return
 *      a clear failure result.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you provide a unified high-level interface to a set of interfaces in a subsystem,
 * making the subsystem easier to use without hiding the low-level classes from advanced clients?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Subsystems (Complex internal microservices)
// ----------------------------------------------------------------------------------------------

class InventoryService {
public:
    bool reserveStock(const std::string& itemId, int qty) {
        if (itemId == "OUT_OF_STOCK_ITEM") return false;
        std::cout << "  [Inventory] Reserved " << qty << " unit(s) of " << itemId << "\n";
        return true;
    }
};

class TaxService {
public:
    double calculateTax(double subtotal, const std::string& zipCode) {
        if (zipCode.rfind("981", 0) == 0) return subtotal * 0.10; // 10% WA tax
        return subtotal * 0.05; // 5% default
    }
};

class PaymentService {
public:
    bool chargeCard(const std::string& customerId, double totalAmount) {
        if (customerId == "CUST_BAD_CREDIT") return false;
        std::cout << "  [Payment] Successfully charged $" << totalAmount << " to customer " << customerId << "\n";
        return true;
    }
};

class ShippingLogisticsService {
public:
    std::string scheduleDelivery(const std::string& customerId, const std::string& itemId) {
        (void)customerId;
        (void)itemId;
        return "TRACK-AMZN-99214";
    }
};

// ----------------------------------------------------------------------------------------------
// Facade Interface & Implementation (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

struct OrderConfirmation {
    bool success{false};
    std::string orderId;
    std::string trackingNumber;
    double finalAmountCharged{0.0};
    std::string failureReason;
};

class OneClickCheckoutFacade {
private:
    std::unique_ptr<InventoryService> inventory_;
    std::unique_ptr<TaxService> tax_;
    std::unique_ptr<PaymentService> payment_;
    std::unique_ptr<ShippingLogisticsService> shipping_;

public:
    OneClickCheckoutFacade()
        : inventory_(std::make_unique<InventoryService>()),
          tax_(std::make_unique<TaxService>()),
          payment_(std::make_unique<PaymentService>()),
          shipping_(std::make_unique<ShippingLogisticsService>()) {}

    // TODO: Implement orchestrator method
    OrderConfirmation checkout(const std::string& customerId,
                               const std::string& itemId,
                               int quantity,
                               double unitPrice,
                               const std::string& zipCode) {
        // TODO:
        // 1. Reserve inventory; if fails, return failure
        // 2. Compute tax on (unitPrice * quantity)
        // 3. Charge payment; if fails, return failure
        // 4. Schedule delivery and retrieve tracking ID
        // 5. Return successful OrderConfirmation
        (void)customerId; (void)itemId; (void)quantity; (void)unitPrice; (void)zipCode;
        return OrderConfirmation{.success = false, .failureReason = "Not Implemented"};
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running 1-Click Checkout Facade Test Suite <<<\n\n";

    OneClickCheckoutFacade checkoutEngine;

    // Test Case 1: Successful 1-Click Purchase
    {
        std::cout << "[Test 1] Successful 1-Click Checkout... \n";
        OrderConfirmation confirmation = checkoutEngine.checkout(
            "CUST_PRIME_42",
            "ITEM_KINDLE_PAPERWHITE",
            1,
            100.00,
            "98101" // WA state tax 10% -> Total $110.00
        );

        assert(confirmation.success && "Checkout should succeed");
        assert(confirmation.finalAmountCharged == 110.00);
        assert(confirmation.trackingNumber == "TRACK-AMZN-99214");
        std::cout << "PASSED (Tracking: " << confirmation.trackingNumber << ")\n";
    }

    // Test Case 2: Out of Stock Short-Circuit
    {
        std::cout << "[Test 2] Out of Stock Handling... \n";
        OrderConfirmation confirmation = checkoutEngine.checkout(
            "CUST_PRIME_42",
            "OUT_OF_STOCK_ITEM",
            1,
            50.00,
            "98101"
        );

        assert(!confirmation.success && "Should fail when item is out of stock");
        assert(confirmation.failureReason.find("Inventory") != std::string::npos ||
               confirmation.failureReason.find("stock") != std::string::npos);
        std::cout << "PASSED\n";
    }

    // Test Case 3: Payment Declined Handling
    {
        std::cout << "[Test 3] Payment Declined Short-Circuit... \n";
        OrderConfirmation confirmation = checkoutEngine.checkout(
            "CUST_BAD_CREDIT",
            "ITEM_ECHO_DOT",
            2,
            25.00,
            "90210"
        );

        assert(!confirmation.success && "Should fail when card charge fails");
        std::cout << "PASSED\n";
    }

    std::cout << "\n🎉 ALL FACADE TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
