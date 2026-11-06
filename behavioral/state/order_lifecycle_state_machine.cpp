/*
 * Order Lifecycle State Machine
 *
 * Requirements:
 * - Encapsulate state-dependent behavior in dedicated IOrderState implementations
 *   (Created, Paid, Shipped, Delivered, Cancelled).
 * - Enforce valid transitions:
 *   - Created -> Paid or Cancelled.
 *   - Paid -> Shipped or Cancelled.
 *   - Shipped -> Delivered (cancellation disallowed).
 *   - Delivered / Cancelled -> terminal states.
 * - Disallowed actions must throw std::logic_error.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cassert>
#include <stdexcept>

class OrderContext; // Forward declaration

// ----------------------------------------------------------------------------------------------
// State Interface (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class IOrderState {
public:
    virtual ~IOrderState() = default;

    virtual void pay(OrderContext& context) = 0;
    virtual void ship(OrderContext& context) = 0;
    virtual void deliver(OrderContext& context) = 0;
    virtual void cancel(OrderContext& context) = 0;
    virtual std::string_view getStateName() const noexcept = 0;
};

// ----------------------------------------------------------------------------------------------
// Context: Order
// ----------------------------------------------------------------------------------------------

class OrderContext {
private:
    std::string orderId_;
    std::unique_ptr<IOrderState> currentState_;

public:
    OrderContext(std::string orderId, std::unique_ptr<IOrderState> initialState)
        : orderId_(std::move(orderId)), currentState_(std::move(initialState)) {}

    void transitionTo(std::unique_ptr<IOrderState> newState) {
        currentState_ = std::move(newState);
    }

    void pay() {
        if (currentState_) currentState_->pay(*this);
    }

    void ship() {
        if (currentState_) currentState_->ship(*this);
    }

    void deliver() {
        if (currentState_) currentState_->deliver(*this);
    }

    void cancel() {
        if (currentState_) currentState_->cancel(*this);
    }

    std::string_view getCurrentStateName() const noexcept {
        return currentState_ ? currentState_->getStateName() : "Unknown";
    }

    std::string_view getOrderId() const noexcept { return orderId_; }
};

// ----------------------------------------------------------------------------------------------
// Concrete States (TODO: Implement state behaviors and transitions)
// ----------------------------------------------------------------------------------------------

class CreatedState : public IOrderState {
public:
    void pay([[maybe_unused]] OrderContext& context) override {
        // TODO: Transition context to PaidState
    }

    void ship([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Cannot ship an unpaid order");
    }

    void deliver([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Cannot deliver an unpaid order");
    }

    void cancel([[maybe_unused]] OrderContext& context) override {
        // TODO: Transition context to CancelledState
    }

    std::string_view getStateName() const noexcept override { return "CREATED"; }
};

class PaidState : public IOrderState {
public:
    void pay([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Order is already paid");
    }

    void ship([[maybe_unused]] OrderContext& context) override {
        // TODO: Transition context to ShippedState
    }

    void deliver([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Order must be shipped before delivery");
    }

    void cancel([[maybe_unused]] OrderContext& context) override {
        // TODO: Transition context to CancelledState (with refund trigger)
    }

    std::string_view getStateName() const noexcept override { return "PAID"; }
};

class ShippedState : public IOrderState {
public:
    void pay([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Already paid and shipped");
    }

    void ship([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Already shipped");
    }

    void deliver([[maybe_unused]] OrderContext& context) override {
        // TODO: Transition context to DeliveredState
    }

    void cancel([[maybe_unused]] OrderContext& context) override {
        throw std::logic_error("Cannot cancel order once shipped! Must request return upon delivery.");
    }

    std::string_view getStateName() const noexcept override { return "SHIPPED"; }
};

class DeliveredState : public IOrderState {
public:
    void pay([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Terminal state"); }
    void ship([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Terminal state"); }
    void deliver([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Terminal state"); }
    void cancel([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Terminal state"); }
    std::string_view getStateName() const noexcept override { return "DELIVERED"; }
};

class CancelledState : public IOrderState {
public:
    void pay([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Order cancelled"); }
    void ship([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Order cancelled"); }
    void deliver([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Order cancelled"); }
    void cancel([[maybe_unused]] OrderContext& context) override { throw std::logic_error("Already cancelled"); }
    std::string_view getStateName() const noexcept override { return "CANCELLED"; }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running order lifecycle state machine tests...\n\n";

    // Test Case 1: Happy Path (Created -> Paid -> Shipped -> Delivered)
    {
        std::cout << "[Test 1] Order Happy Path Progression... ";
        OrderContext order("ORD-101", std::make_unique<CreatedState>());
        assert(order.getCurrentStateName() == "CREATED");

        order.pay();
        assert(order.getCurrentStateName() == "PAID");

        order.ship();
        assert(order.getCurrentStateName() == "SHIPPED");

        order.deliver();
        assert(order.getCurrentStateName() == "DELIVERED");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Illegal Transition (Cannot ship an unpaid order)
    {
        std::cout << "[Test 2] Illegal Transition Guard (Ship unpaid order)... ";
        OrderContext order("ORD-102", std::make_unique<CreatedState>());

        bool caught = false;
        try {
            order.ship();
        } catch (const std::logic_error&) {
            caught = true;
        }
        assert(caught && "Shipping unpaid order must throw logic_error");
        assert(order.getCurrentStateName() == "CREATED");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Invariant Protection (Cannot cancel shipped package)
    {
        std::cout << "[Test 3] Illegal Cancellation Guard (Cancel shipped order)... ";
        OrderContext order("ORD-103", std::make_unique<CreatedState>());
        order.pay();
        order.ship();
        assert(order.getCurrentStateName() == "SHIPPED");

        bool caught = false;
        try {
            order.cancel();
        } catch (const std::logic_error&) {
            caught = true;
        }
        assert(caught && "Cancelling shipped order must throw logic_error");
        assert(order.getCurrentStateName() == "SHIPPED");
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll order lifecycle state machine tests passed.\n";
    return 0;
}
