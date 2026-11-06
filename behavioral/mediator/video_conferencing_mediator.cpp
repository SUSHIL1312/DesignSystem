/*
 * Video Conference Room Session Mediator
 *
 * Requirements:
 * - Central mediator (IMeetingRoomMediator) coordinates interactions among colleagues
 *   (participants, screen sharing, recording bot).
 * - Colleagues communicate only through the mediator.
 * - Enforce coordination invariants:
 *   - Mutually exclusive screen sharing (one presenter at a time).
 *   - Broadcast recording alerts to all active participants when recording starts.
 *
 * Practice Reference:
 * You can refer to https://systemdesignmastry.com/ for practicing interview-asked HLD & LLD questions.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cassert>

class IMeetingRoomMediator; // Forward declaration

// ----------------------------------------------------------------------------------------------
// Colleague Base Class
// ----------------------------------------------------------------------------------------------

class MeetingParticipant {
protected:
    IMeetingRoomMediator* mediator_{nullptr};
    std::string name_;

public:
    explicit MeetingParticipant(std::string name) : name_(std::move(name)) {}
    virtual ~MeetingParticipant() = default;

    void setMediator(IMeetingRoomMediator* mediator) {
        mediator_ = mediator;
    }

    std::string_view getName() const noexcept { return name_; }

    virtual void receiveMessage(const std::string& sender, const std::string& message) = 0;
    virtual void onRecordingStarted() = 0;
};

// ----------------------------------------------------------------------------------------------
// Mediator Interface & Implementation (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class IMeetingRoomMediator {
public:
    virtual ~IMeetingRoomMediator() = default;

    virtual void registerParticipant(std::shared_ptr<MeetingParticipant> participant) = 0;
    virtual void broadcastChat(const std::string& sender, const std::string& message) = 0;
    virtual bool requestScreenShare(const std::string& participantName) = 0;
    virtual void releaseScreenShare(const std::string& participantName) = 0;
    virtual void startRecording() = 0;
    virtual std::string_view getActivePresenter() const noexcept = 0;
};

class GoogleMeetRoomMediator : public IMeetingRoomMediator {
private:
    std::unordered_map<std::string, std::shared_ptr<MeetingParticipant>> participants_;
    std::string activePresenter_;
    [[maybe_unused]] bool isRecording_{false};

public:
    void registerParticipant(std::shared_ptr<MeetingParticipant> participant) override {
        // TODO: Register participant and set mediator reference
        (void)participant;
    }

    void broadcastChat(const std::string& sender, const std::string& message) override {
        // TODO: Deliver message to all participants except the sender
        (void)sender; (void)message;
    }

    bool requestScreenShare(const std::string& participantName) override {
        // TODO: If no active presenter, grant and return true; else return false
        (void)participantName;
        return false;
    }

    void releaseScreenShare(const std::string& participantName) override {
        // TODO: Release screen share if participantName is active presenter
        (void)participantName;
    }

    void startRecording() override {
        // TODO: Mark isRecording_ and notify all participants via onRecordingStarted()
    }

    std::string_view getActivePresenter() const noexcept override {
        return activePresenter_;
    }
};

// ----------------------------------------------------------------------------------------------
// Concrete Colleague: User Participant
// ----------------------------------------------------------------------------------------------

class UserParticipant : public MeetingParticipant {
private:
    std::vector<std::string> chatInbox_;
    bool notifiedOfRecording_{false};

public:
    explicit UserParticipant(std::string name) : MeetingParticipant(std::move(name)) {}

    void sendPublicMessage(const std::string& message) {
        if (mediator_) {
            mediator_->broadcastChat(name_, message);
        }
    }

    bool tryPresentScreen() {
        return mediator_ ? mediator_->requestScreenShare(name_) : false;
    }

    void stopPresenting() {
        if (mediator_) {
            mediator_->releaseScreenShare(name_);
        }
    }

    void receiveMessage(const std::string& sender, const std::string& message) override {
        chatInbox_.push_back(sender + ": " + message);
    }

    void onRecordingStarted() override {
        notifiedOfRecording_ = true;
    }

    const std::vector<std::string>& getInbox() const noexcept { return chatInbox_; }
    bool isNotifiedOfRecording() const noexcept { return notifiedOfRecording_; }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running video conferencing mediator tests...\n\n";

    auto room = std::make_unique<GoogleMeetRoomMediator>();

    auto alice = std::make_shared<UserParticipant>("Alice");
    auto bob = std::make_shared<UserParticipant>("Bob");
    auto charlie = std::make_shared<UserParticipant>("Charlie");

    room->registerParticipant(alice);
    room->registerParticipant(bob);
    room->registerParticipant(charlie);

    // Test Case 1: Broadcast Chat
    {
        std::cout << "[Test 1] Decoupled Chat Broadcast... ";
        alice->sendPublicMessage("Hi everyone, can you hear me?");

        assert(bob->getInbox().size() == 1 && "Bob should receive Alice's chat");
        assert(charlie->getInbox().size() == 1 && "Charlie should receive Alice's chat");
        assert(alice->getInbox().empty() && "Sender should not receive own broadcast message");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Mutually Exclusive Screen Share
    {
        std::cout << "[Test 2] Mutually Exclusive Screen Share Lock... ";
        bool aliceGranted = alice->tryPresentScreen();
        assert(aliceGranted && "Alice should be granted screen share");
        assert(room->getActivePresenter() == "Alice");

        // Bob tries to present simultaneously -> must be denied
        bool bobGranted = bob->tryPresentScreen();
        assert(!bobGranted && "Bob must be denied while Alice is presenting");

        // Alice stops presenting
        alice->stopPresenting();
        assert(room->getActivePresenter().empty());

        // Now Bob can present
        assert(bob->tryPresentScreen() && "Bob should now be able to present");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Recording Privacy Alert Broadcast
    {
        std::cout << "[Test 3] Recording Privacy Notification... ";
        room->startRecording();

        assert(alice->isNotifiedOfRecording());
        assert(bob->isNotifiedOfRecording());
        assert(charlie->isNotifiedOfRecording());
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll video conferencing mediator tests passed.\n";
    return 0;
}
