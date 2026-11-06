/*
 * Social Graph BFS Traversal Iterator
 *
 * Requirements:
 * - Encapsulate graph traversal behind a generic IIterator<T> interface (hasNext, next).
 * - Traverse graph connections level-by-level (Breadth-First Search) up to maxDepth hops.
 * - Handle cycles correctly without duplicate visits or infinite loops.
 * - Graph internal storage (adjacency list, visited set) remains private.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <cassert>
#include <stdexcept>

// ----------------------------------------------------------------------------------------------
// Domain Models
// ----------------------------------------------------------------------------------------------

struct UserProfile {
    std::string userId;
    std::string name;

    bool operator==(const UserProfile& other) const {
        return userId == other.userId;
    }
};

// ----------------------------------------------------------------------------------------------
// Iterator Interface (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

template <typename T>
class IIterator {
public:
    virtual ~IIterator() = default;

    virtual bool hasNext() = 0;
    virtual T next() = 0;
};

// ----------------------------------------------------------------------------------------------
// Aggregate & Concrete BFS Iterator
// ----------------------------------------------------------------------------------------------

class SocialGraph {
private:
    std::unordered_map<std::string, UserProfile> profiles_;
    std::unordered_map<std::string, std::vector<std::string>> adjacencyList_;

public:
    void addUser(std::string id, std::string name) {
        profiles_[id] = UserProfile{.userId = id, .name = std::move(name)};
    }

    void addConnection(const std::string& u1, const std::string& u2) {
        adjacencyList_[u1].push_back(u2);
        adjacencyList_[u2].push_back(u1); // Undirected graph
    }

    const UserProfile* getProfile(const std::string& id) const {
        auto it = profiles_.find(id);
        return it != profiles_.end() ? &it->second : nullptr;
    }

    const std::vector<std::string>& getNeighbors(const std::string& id) const {
        static const std::vector<std::string> empty;
        auto it = adjacencyList_.find(id);
        return it != adjacencyList_.end() ? it->second : empty;
    }

    // Concrete BFS Iterator
    class BfsIterator : public IIterator<UserProfile> {
    private:
        [[maybe_unused]] const SocialGraph& graph_;
        [[maybe_unused]] int maxDepth_;
        std::queue<std::pair<std::string, int>> queue_; // {userId, currentDepth}
        std::unordered_set<std::string> visited_;

    public:
        BfsIterator(const SocialGraph& graph, const std::string& startUserId, int maxDepth)
            : graph_(graph), maxDepth_(maxDepth) {
            // TODO: Initialize BFS starting node and visited set
            (void)startUserId;
        }

        bool hasNext() override {
            // TODO: Return true if queue has unvisited valid nodes within maxDepth
            return !queue_.empty();
        }

        UserProfile next() override {
            if (!hasNext()) {
                throw std::out_of_range("No more elements in iterator");
            }
            // TODO: Pop front node, expand unvisited neighbors into queue (if depth + 1 <= maxDepth_), return profile
            return UserProfile{};
        }
    };

    std::unique_ptr<IIterator<UserProfile>> createBfsIterator(const std::string& startUserId, int maxDepth) const {
        return std::make_unique<BfsIterator>(*this, startUserId, maxDepth);
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running social graph BFS iterator tests...\n\n";

    // Setup graph:
    // Alice (root) -> Bob, Charlie
    // Bob -> Dave
    // Charlie -> Dave (cycle test!)
    // Dave -> Eve (3rd degree)
    SocialGraph graph;
    graph.addUser("u1", "Alice");
    graph.addUser("u2", "Bob");
    graph.addUser("u3", "Charlie");
    graph.addUser("u4", "Dave");
    graph.addUser("u5", "Eve");

    graph.addConnection("u1", "u2"); // Alice - Bob
    graph.addConnection("u1", "u3"); // Alice - Charlie
    graph.addConnection("u2", "u4"); // Bob - Dave
    graph.addConnection("u3", "u4"); // Charlie - Dave
    graph.addConnection("u4", "u5"); // Dave - Eve

    // Test Case 1: Traversal up to 1 hop (Alice + direct friends: Bob, Charlie)
    {
        std::cout << "[Test 1] 1-Hop BFS Traversal... ";
        auto it = graph.createBfsIterator("u1", 1);

        std::vector<std::string> visitedNames;
        while (it->hasNext()) {
            visitedNames.push_back(it->next().name);
        }

        assert(visitedNames.size() == 3 && "Should yield Alice, Bob, and Charlie");
        assert(visitedNames[0] == "Alice");
        std::cout << "PASSED (Yielded 3 profiles)\n";
    }

    // Test Case 2: Traversal up to 2 hops with Cycle Handling (Alice -> Bob, Charlie -> Dave)
    {
        std::cout << "[Test 2] 2-Hop BFS with Cycle Deduplication... ";
        auto it = graph.createBfsIterator("u1", 2);

        std::vector<std::string> visitedNames;
        while (it->hasNext()) {
            visitedNames.push_back(it->next().name);
        }

        // Expected: Alice, Bob, Charlie, Dave (Dave visited only once despite 2 paths!)
        assert(visitedNames.size() == 4 && "Cycle must not cause duplicate Dave visit");
        assert(visitedNames[3] == "Dave");
        std::cout << "PASSED (Cycle handled cleanly without duplicates)\n";
    }

    std::cout << "\nAll social graph BFS iterator tests passed.\n";
    return 0;
}
