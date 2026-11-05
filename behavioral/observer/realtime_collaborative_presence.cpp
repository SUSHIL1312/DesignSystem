/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Google Docs Real-Time Collaborative Presence & Edit Broker
 * Category: Behavioral Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * In Google Docs, multiple collaborators edit the same document in real time.
 * Whenever a user types a paragraph, moves their cursor, or highlights text:
 *   1. Connected web client sessions must render the remote user's cursor position.
 *   2. An Audit Compliance Service must record document version timestamps.
 *   3. A Spellcheck & AI Grammar Engine must analyze newly inserted text chunks.
 * 
 * Hardcoding notifications inside the core `DocumentEditingEngine` would tightly couple
 * text editing to network sessions, logging, and AI pipelines.
 * 
 * The system requires an event broker where disparate listeners can subscribe and unsubscribe
 * dynamically without the document engine knowing who is listening.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Decoupled Publisher / Subscriber:
 *    - `IDocumentObserver` defines the update contract: `onDocumentEvent(const DocumentEvent&)`.
 *    - `DocumentSessionBroker` coordinates observer registration, unregistration, and broadcasting.
 * 2. Dynamic Lifecycle & Safety:
 *    - Collaborators can join and leave at any instant.
 *    - Unsubscribing must prevent further notifications and avoid dangling references.
 * 3. Event Types & Payload Filtering:
 *    - Events: `CURSOR_MOVED`, `TEXT_INSERTED`, `COMMENT_ADDED`.
 * 4. Thread-Safety:
 *    - Observer list mutations and event broadcasting must be thread-safe.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you define a one-to-many dependency between objects so that when one object changes state,
 * all its dependents are notified and updated automatically?
 * ==============================================================================================
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
    std::cout << ">>> Running Google Docs Presence Broker (Observer) Test Suite <<<\n\n";

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

    std::cout << "\n🎉 ALL OBSERVER TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
