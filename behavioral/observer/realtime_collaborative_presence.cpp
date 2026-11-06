/*
 * Real-Time Collaborative Presence & Event Broker
 *
 * Requirements:
 * - Observer interface (IDocumentObserver) and broker (DocumentSessionBroker).
 * - Decouple document event generation from listeners (UI cursor renderers, audit logs).
 * - Support dynamic subscription and unsubscription of listeners.
 * - Thread-safe listener registration and event broadcasting.
 *
 * Practice Reference:
 * You can refer to https://systemdesignmastry.com/ for practicing interview-asked HLD & LLD questions.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Domain Models
// ----------------------------------------------------------------------------------------------

enum class EventType {
    CURSOR_MOVED,
    TEXT_INSERTED,
    COMMENT_ADDED
};

struct DocumentEvent {
    std::string docId;
    std::string authorUserId;
    EventType type;
    std::string content; // inserted text or cursor coordinates "(x=10, y=50)"
    uint64_t timestamp;
};

// ----------------------------------------------------------------------------------------------
// Observer Interface (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class IDocumentObserver {
public:
    virtual ~IDocumentObserver() = default;

    // TODO: Define the notification receive contract
    virtual void onDocumentEvent(const DocumentEvent& event) = 0;
    virtual std::string_view getObserverId() const noexcept = 0;
};

// ----------------------------------------------------------------------------------------------
// Concrete Observers
// ----------------------------------------------------------------------------------------------

class ClientCursorRenderer : public IDocumentObserver {
private:
    std::string clientId_;
    std::string lastRenderedPos_;

public:
    explicit ClientCursorRenderer(std::string clientId) : clientId_(std::move(clientId)) {}

    void onDocumentEvent(const DocumentEvent& event) override {
        // TODO: Update lastRenderedPos_ when event is CURSOR_MOVED
        (void)event;
    }

    std::string_view getObserverId() const noexcept override { return clientId_; }
    std::string_view getLastRenderedPos() const noexcept { return lastRenderedPos_; }
};

class AuditLogService : public IDocumentObserver {
private:
    std::vector<DocumentEvent> eventHistory_;

public:
    void onDocumentEvent(const DocumentEvent& event) override {
        // TODO: Append event to history
        (void)event;
    }

    std::string_view getObserverId() const noexcept override { return "AuditLogService"; }
    size_t getLoggedEventsCount() const noexcept { return eventHistory_.size(); }
};

// ----------------------------------------------------------------------------------------------
// Subject / Publisher (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class DocumentSessionBroker {
private:
    // TODO: Manage registered observers in a thread-safe manner
    std::unordered_map<std::string, std::shared_ptr<IDocumentObserver>> observers_;
    mutable std::mutex mtx_;

public:
    void subscribe(std::shared_ptr<IDocumentObserver> observer) {
        // TODO: Register observer
        (void)observer;
    }

    void unsubscribe(const std::string& observerId) {
        // TODO: Remove observer by ID
        (void)observerId;
    }

    void broadcast(const DocumentEvent& event) {
        // TODO: Notify all registered observers
        (void)event;
    }

    size_t getObserverCount() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return observers_.size();
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running collaborative presence broker tests...\n\n";

    DocumentSessionBroker broker;

    auto aliceClient = std::make_shared<ClientCursorRenderer>("Alice_UI");
    auto bobClient = std::make_shared<ClientCursorRenderer>("Bob_UI");
    auto auditLogger = std::make_shared<AuditLogService>();

    // Test Case 1: Subscription & Event Broadcasting
    {
        std::cout << "[Test 1] Subscribe and Broadcast... ";
        broker.subscribe(aliceClient);
        broker.subscribe(bobClient);
        broker.subscribe(auditLogger);
        assert(broker.getObserverCount() == 3);

        DocumentEvent cursorEvent{
            .docId = "doc_design_doc_101",
            .authorUserId = "user_charlie",
            .type = EventType::CURSOR_MOVED,
            .content = "line=42,col=15",
            .timestamp = 1699990000
        };

        broker.broadcast(cursorEvent);

        assert(aliceClient->getLastRenderedPos() == "line=42,col=15");
        assert(bobClient->getLastRenderedPos() == "line=42,col=15");
        assert(auditLogger->getLoggedEventsCount() == 1);
        std::cout << "PASSED\n";
    }

    // Test Case 2: Unsubscription Lifecycle
    // Bob closes his browser tab -> Unsubscribes
    {
        std::cout << "[Test 2] Observer Unsubscription... ";
        broker.unsubscribe("Bob_UI");
        assert(broker.getObserverCount() == 2);

        DocumentEvent editEvent{
            .docId = "doc_design_doc_101",
            .authorUserId = "user_alice",
            .type = EventType::TEXT_INSERTED,
            .content = "Added section 3.2",
            .timestamp = 1699990050
        };

        broker.broadcast(editEvent);

        // Audit logger should have 2 events total now
        assert(auditLogger->getLoggedEventsCount() == 2);
        // Bob shouldn't receive updates anymore
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll collaborative presence broker tests passed.\n";
    return 0;
}
