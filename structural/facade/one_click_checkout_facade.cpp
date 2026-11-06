/*
 * One-Click Checkout Subsystem Facade
 *
 * Requirements:
 * - Provide a simplified, high-level interface for order purchases.
 * - Coordinate inventory reservation, tax calculation, payment billing, and shipping.
 * - Short-circuit on failure (e.g. out of stock, payment declined) and return descriptive status.
 * - Subsystems remain decoupled and accessible for advanced fulfillment workflows.
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
    std::cout << "Running one-click checkout facade tests...\n\n";

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

    std::cout << "\nAll one-click checkout facade tests passed.\n";
    return 0;
}
