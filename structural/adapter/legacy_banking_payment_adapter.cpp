/*
 * Payment Gateway Adapter for Legacy Banking Service
 *
 * Requirements:
 * - Adapt legacy banking API (LegacySoapBankService) to modern IPaymentGateway interface.
 * - Protocol & data translation:
 *   - Convert dollars to integer cents.
 *   - Manage legacy session tokens.
 *   - Map legacy integer return codes (1, 0, -1) to structured PaymentResult.
 * - Use composition to wrap the legacy service.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <cmath>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Modern Target Interface & Domain Models
// ----------------------------------------------------------------------------------------------

struct PaymentRequest {
    std::string customerId;
    double amountDollars{0.0};
    std::string currency;
    std::string cardToken;
};

struct PaymentResult {
    bool success{false};
    std::string transactionReference;
    std::string message;
};

class IPaymentGateway {
public:
    virtual ~IPaymentGateway() = default;
    virtual PaymentResult processPayment(const PaymentRequest& request) = 0;
};

// ----------------------------------------------------------------------------------------------
// Incompatible Adaptee: Third-Party Legacy Banking SDK (Do NOT modify this class)
// ----------------------------------------------------------------------------------------------

class LegacySoapBankService {
public:
    int openSession(const std::string& merchantKey) {
        if (merchantKey == "AMAZON_SECURE_KEY") return 98765; // Session token
        return 0; // Failed
    }

    int executeXmlTransaction(int sessionToken, const std::string& cardHash, int amountInCents) {
        if (sessionToken != 98765) return -2; // Invalid session
        if (amountInCents <= 0) return -3;
        if (cardHash == "CARD_FRAUD_TRIGGER") return -1; // Fraud rejected
        if (amountInCents > 500000) return 0; // Insufficient funds (limit $5000)
        return 1; // Success
    }

    std::string getLastBankTraceId() {
        return "SOAP-BANK-REF-884920";
    }
};

// ----------------------------------------------------------------------------------------------
// Adapter Implementation (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class LegacyBankPaymentAdapter : public IPaymentGateway {
private:
    std::unique_ptr<LegacySoapBankService> legacyService_;
    std::string merchantKey_;

public:
    explicit LegacyBankPaymentAdapter(std::unique_ptr<LegacySoapBankService> legacyService,
                                     std::string merchantKey)
        : legacyService_(std::move(legacyService)), merchantKey_(std::move(merchantKey)) {}

    PaymentResult processPayment(const PaymentRequest& request) override {
        // TODO:
        // 1. Convert amountDollars to integer cents (e.g. std::round(amount * 100.0))
        // 2. Open legacy session using merchantKey_
        // 3. Execute legacy transaction
        // 4. Map return code (1 -> success, 0 -> Insufficient funds, -1 -> Fraud)
        // 5. Populate and return PaymentResult
        (void)request;
        return PaymentResult{.success = false, .message = "Not Implemented"};
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running legacy banking payment adapter tests...\n\n";

    auto legacyBank = std::make_unique<LegacySoapBankService>();
    std::unique_ptr<IPaymentGateway> paymentGateway = 
        std::make_unique<LegacyBankPaymentAdapter>(std::move(legacyBank), "AMAZON_SECURE_KEY");

    // Test Case 1: Successful Payment Translation ($49.99 -> 4999 cents)
    {
        std::cout << "[Test 1] Successful Payment Translation... ";
        PaymentRequest req{
            .customerId = "cust_101",
            .amountDollars = 49.99,
            .currency = "USD",
            .cardToken = "CARD_VALID_4242"
        };

        PaymentResult res = paymentGateway->processPayment(req);
        assert(res.success && "Payment should succeed for valid credentials");
        assert(!res.transactionReference.empty() && "Should capture legacy bank trace ID");
        std::cout << "PASSED (Trace: " << res.transactionReference << ")\n";
    }

    // Test Case 2: Fraud Detection Mapping
    {
        std::cout << "[Test 2] Fraud Status Mapping... ";
        PaymentRequest req{
            .customerId = "cust_fraud",
            .amountDollars = 150.00,
            .currency = "USD",
            .cardToken = "CARD_FRAUD_TRIGGER"
        };

        PaymentResult res = paymentGateway->processPayment(req);
        assert(!res.success && "Fraud trigger card must be declined");
        assert(res.message.find("Fraud") != std::string::npos || res.message.find("Declined") != std::string::npos);
        std::cout << "PASSED (Message: " << res.message << ")\n";
    }

    // Test Case 3: Insufficient Funds Mapping ($6,000 > limit)
    {
        std::cout << "[Test 3] Over-limit Insufficient Funds... ";
        PaymentRequest req{
            .customerId = "cust_high",
            .amountDollars = 6000.00,
            .currency = "USD",
            .cardToken = "CARD_VALID_4242"
        };

        PaymentResult res = paymentGateway->processPayment(req);
        assert(!res.success && "Amounts over $5,000 must be declined");
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll legacy banking payment adapter tests passed.\n";
    return 0;
}
