/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Amazon Multi-Channel Notification Dispatcher (Amazon SNS / Pinpoint Engine)
 * Category: Creational Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * At Amazon scale, core microservices (Order Fulfillment, CloudWatch Alarms, Prime Delivery)
 * emit billions of notifications daily. Each notification must be routed through specific
 * transport channels depending on customer preferences and message urgency:
 *   1. Email (AWS SES): Formats HTML/Plaintext, validates RFC 5322 emails, handles DKIM signing.
 *   2. SMS (AWS Pinpoint): Validates E.164 phone numbers, enforces 160-char SMS segmenting.
 *   3. Push (FCM/APNs): Validates hex/base64 device tokens, constructs platform-specific payloads.
 * 
 * The platform dispatch pipeline must operate agnostically: it handles rate-limiting, retries,
 * and delivery audits, but must NEVER know the concrete instantiation details, protocol
 * handshakes, or endpoint configs of individual channels.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Single Responsibility Principle (SRP):
 *    - Separation between channel payload formatting/transmission and channel creation.
 * 2. Open-Closed Principle (OCP):
 *    - Adding a brand-new notification channel (e.g., Slack Webhook, WhatsApp) must require
 *      ZERO changes to existing channel classes or core dispatcher logic.
 *    - Must support runtime or compile-time registration of new creators.
 * 3. Modern C++ Memory & Ownership:
 *    - Clean ownership semantics (`std::unique_ptr`, `std::string_view` where appropriate).
 *    - Virtual destructors on polymorphic bases; Rule of 5 adherence.
 * 4. Resilient Error Handling:
 *    - Malformed recipient/payload must return a failed `DeliveryResult` with diagnostic info,
 *      not crash or throw unhandled exceptions.
 * 5. Thread-Safety:
 *    - Multiple worker threads dispatching notifications concurrently must not cause race conditions.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * Instead of letting your client code or a monolithic factory switch over an enum (`switch (type)`),
 * invert the creation dependency. Define an interface for the product, and an abstract creation
 * contract where derived creators decide which concrete product to instantiate.
 * How can you make this open for registration so adding a 4th or 5th channel requires NO edits to existing files?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <cassert>
#include <chrono>

// ----------------------------------------------------------------------------------------------
// Domain Models (Pre-defined for consistency)
// ----------------------------------------------------------------------------------------------

enum class NotificationPriority {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct NotificationPayload {
    std::string recipient;      // email, phone (+1...), or device_token
    std::string subject;
    std::string body;
    NotificationPriority priority{NotificationPriority::MEDIUM};
};

struct DeliveryResult {
    bool success{false};
    std::string messageId;
    std::string channelName;
    std::string errorMessage;
};

// ----------------------------------------------------------------------------------------------
// Skeletons & Signatures (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

/**
 * Abstract Product: Represents a generic notification transmission channel.
 */
class INotificationChannel {
public:
    virtual ~INotificationChannel() = default;

    // TODO: Define the core contract for sending notifications.
    virtual DeliveryResult send(const NotificationPayload& payload) = 0;
    virtual std::string_view getChannelName() const noexcept = 0;
};

/**
 * Concrete Product: Email Channel (AWS SES simulator)
 * Requirements:
 * - Recipient must contain '@' and '.'
 * - MessageId should be prefixed with "EMAIL-"
 */
class EmailNotificationChannel : public INotificationChannel {
public:
    // TODO: Implement your design here
    DeliveryResult send(const NotificationPayload& payload) override {
        // TODO: Validate recipient and return DeliveryResult
        (void)payload;
        return DeliveryResult{.success = false, .errorMessage = "Not Implemented"};
    }

    std::string_view getChannelName() const noexcept override {
        return "EMAIL";
    }
};

/**
 * Concrete Product: SMS Channel (AWS Pinpoint simulator)
 * Requirements:
 * - Recipient must start with '+' and have at least 10 digits
 * - Body over 160 chars should either be truncated or flagged
 * - MessageId should be prefixed with "SMS-"
 */
class SMSNotificationChannel : public INotificationChannel {
public:
    // TODO: Implement your design here
    DeliveryResult send(const NotificationPayload& payload) override {
        // TODO: Validate recipient and return DeliveryResult
        (void)payload;
        return DeliveryResult{.success = false, .errorMessage = "Not Implemented"};
    }

    std::string_view getChannelName() const noexcept override {
        return "SMS";
    }
};

/**
 * Concrete Product: Push Channel (FCM/APNs simulator)
 * Requirements:
 * - Recipient must be a non-empty token string (min 16 chars)
 * - MessageId should be prefixed with "PUSH-"
 */
class PushNotificationChannel : public INotificationChannel {
public:
    // TODO: Implement your design here
    DeliveryResult send(const NotificationPayload& payload) override {
        // TODO: Validate recipient and return DeliveryResult
        (void)payload;
        return DeliveryResult{.success = false, .errorMessage = "Not Implemented"};
    }

    std::string_view getChannelName() const noexcept override {
        return "PUSH";
    }
};

/**
 * Abstract Creator / Factory Interface
 * Declares the factory method that returns a new INotificationChannel object.
 */
class ChannelCreator {
public:
    virtual ~ChannelCreator() = default;

    // Factory Method
    virtual std::unique_ptr<INotificationChannel> createChannel() const = 0;
};

// TODO: Implement Concrete Creators:
// - EmailChannelCreator
// - SMSChannelCreator
// - PushChannelCreator

class EmailChannelCreator : public ChannelCreator {
public:
    std::unique_ptr<INotificationChannel> createChannel() const override {
        // TODO: Return configured EmailNotificationChannel
        return nullptr;
    }
};

class SMSChannelCreator : public ChannelCreator {
public:
    std::unique_ptr<INotificationChannel> createChannel() const override {
        // TODO: Return configured SMSNotificationChannel
        return nullptr;
    }
};

class PushChannelCreator : public ChannelCreator {
public:
    std::unique_ptr<INotificationChannel> createChannel() const override {
        // TODO: Return configured PushNotificationChannel
        return nullptr;
    }
};

/**
 * Channel Registry & Dispatch Service:
 * Manages channel creators dynamically to satisfy the Open-Closed Principle (OCP).
 * Allows dynamic registration of new channels at runtime without touching core code.
 */
class NotificationService {
private:
    // TODO: Manage registered ChannelCreators in a thread-safe manner
    std::unordered_map<std::string, std::unique_ptr<ChannelCreator>> creators_;
    mutable std::mutex registryMutex_;

public:
    void registerChannel(const std::string& channelType, std::unique_ptr<ChannelCreator> creator) {
        // TODO: Register creator in thread-safe registry
        (void)channelType;
        (void)creator;
    }

    DeliveryResult dispatch(const std::string& channelType, const NotificationPayload& payload) {
        // TODO: Lookup creator, create product via factory method, and execute send.
        // Return structured failure if channel is unrecognized.
        (void)channelType;
        (void)payload;
        return DeliveryResult{.success = false, .errorMessage = "Not Implemented"};
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running Notification Dispatcher Test Suite <<<\n\n";

    NotificationService service;

    // Register out-of-the-box channels
    service.registerChannel("EMAIL", std::make_unique<EmailChannelCreator>());
    service.registerChannel("SMS", std::make_unique<SMSChannelCreator>());
    service.registerChannel("PUSH", std::make_unique<PushChannelCreator>());

    // Test Case 1: Standard Email Dispatch
    {
        std::cout << "[Test 1] Valid Email Dispatch... ";
        NotificationPayload emailPayload{
            .recipient = "alex_engineer@amazon.com",
            .subject = "Order #109283 Shipped",
            .body = "<h1>Your package is out for delivery!</h1>",
            .priority = NotificationPriority::HIGH
        };

        DeliveryResult result = service.dispatch("EMAIL", emailPayload);
        assert(result.success && "Email dispatch should succeed for valid address");
        assert(result.channelName == "EMAIL");
        assert(result.messageId.rfind("EMAIL-", 0) == 0 && "Message ID must have EMAIL- prefix");
        std::cout << "PASSED (MsgId: " << result.messageId << ")\n";
    }

    // Test Case 2: Email Malformed Recipient Validation
    {
        std::cout << "[Test 2] Invalid Email Handling... ";
        NotificationPayload badEmail{
            .recipient = "invalid_user_without_at_symbol",
            .subject = "Ping",
            .body = "Hello",
            .priority = NotificationPriority::LOW
        };

        DeliveryResult result = service.dispatch("EMAIL", badEmail);
        assert(!result.success && "Email dispatch must fail for invalid address format");
        assert(!result.errorMessage.empty() && "Error message must be populated");
        std::cout << "PASSED (Rejected as expected: " << result.errorMessage << ")\n";
    }

    // Test Case 3: Valid SMS Dispatch & Formatting
    {
        std::cout << "[Test 3] Valid SMS Dispatch... ";
        NotificationPayload smsPayload{
            .recipient = "+14155552671",
            .subject = "",
            .body = "Your Amazon OTP is: 938421. Valid for 10 minutes.",
            .priority = NotificationPriority::CRITICAL
        };

        DeliveryResult result = service.dispatch("SMS", smsPayload);
        assert(result.success && "SMS dispatch should succeed for valid E.164 phone");
        assert(result.channelName == "SMS");
        assert(result.messageId.rfind("SMS-", 0) == 0 && "Message ID must have SMS- prefix");
        std::cout << "PASSED (MsgId: " << result.messageId << ")\n";
    }

    // Test Case 4: Open-Closed Principle (OCP) Dynamic Extension
    // A 3rd party team adds a SLACK Webhook channel WITHOUT modifying NotificationService or existing channels!
    {
        std::cout << "[Test 4] OCP Verification - Dynamic Slack Channel Registration... ";

        class SlackWebhookChannel : public INotificationChannel {
        public:
            DeliveryResult send(const NotificationPayload& payload) override {
                if (payload.recipient.rfind("https://hooks.slack.com/", 0) != 0) {
                    return DeliveryResult{.success = false, .errorMessage = "Invalid Slack Webhook URL"};
                }
                return DeliveryResult{
                    .success = true,
                    .messageId = "SLACK-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()),
                    .channelName = "SLACK",
                    .errorMessage = ""
                };
            }
            std::string_view getChannelName() const noexcept override {
                return "SLACK";
            }
        };

        class SlackChannelCreator : public ChannelCreator {
        public:
            std::unique_ptr<INotificationChannel> createChannel() const override {
                return std::make_unique<SlackWebhookChannel>();
            }
        };

        // Register new channel dynamically
        service.registerChannel("SLACK", std::make_unique<SlackChannelCreator>());

        NotificationPayload slackPayload{
            .recipient = "https://hooks.slack.com/services/T00/B00/XXXXX",
            .subject = "Prod Alert",
            .body = "CPU utilization exceeded 90% on fleet-us-east-1",
            .priority = NotificationPriority::CRITICAL
        };

        DeliveryResult result = service.dispatch("SLACK", slackPayload);
        assert(result.success && "Dynamically registered Slack channel should work flawlessly");
        assert(result.channelName == "SLACK");
        assert(result.messageId.rfind("SLACK-", 0) == 0);
        std::cout << "PASSED (Dynamically registered channel dispatched successfully!)\n";
    }

    // Test Case 5: Unregistered Channel Query
    {
        std::cout << "[Test 5] Unregistered Channel Lookup... ";
        NotificationPayload payload{.recipient = "target", .subject = "hi", .body = "msg"};
        DeliveryResult result = service.dispatch("DISCORD", payload);
        assert(!result.success && "Unregistered channel should gracefully fail");
        std::cout << "PASSED\n";
    }

    // Test Case 6: Concurrent Dispatch Thread-Safety
    {
        std::cout << "[Test 6] Multi-threaded Concurrent Dispatch... ";
        constexpr int numThreads = 8;
        constexpr int msgsPerThread = 50;
        std::vector<std::thread> workers;

        for (int i = 0; i < numThreads; ++i) {
            workers.emplace_back([&service, i]() {
                for (int j = 0; j < msgsPerThread; ++j) {
                    NotificationPayload p{
                        .recipient = "+12065550" + std::to_string(100 + (j % 900)),
                        .subject = "Ping",
                        .body = "Thread test " + std::to_string(i),
                        .priority = NotificationPriority::LOW
                    };
                    DeliveryResult res = service.dispatch("SMS", p);
                    assert(res.success && "Concurrent SMS dispatch failed");
                }
            });
        }

        for (auto& t : workers) {
            t.join();
        }
        std::cout << "PASSED (" << (numThreads * msgsPerThread) << " concurrent dispatches clean)\n";
    }

    std::cout << "\n🎉 ALL NOTIFICATION DISPATCHER TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
