/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Google / Meta Social Graph Bounded BFS Traversal Iterator
 * Category: Behavioral Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * At Meta / Google, the social graph connects billions of profiles. Features like
 * "People You May Know" and "Shared Circles" traverse a user's network layer by layer:
 *   - Level 0: Self
 *   - Level 1: Immediate friends (1st degree)
 *   - Level 2: Friends of friends (2nd degree)
 * 
 * Exposing the raw adjacency list or internal storage partitions directly to caller algorithms
 * violates encapsulation, prevents concurrent read-isolation, and risks infinite loops on cyclic graphs.
 * 
 * The system requires a clean, standard Iterator that yields profiles in Breadth-First Search (BFS)
 * order up to a specified maximum hop depth, keeping visited sets and queue traversal completely encapsulated.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Encapsulated Traversal:
 *    - `IIterator<T>` contract: `hasNext()` and `next()`.
 *    - Graph internals (adjacency lists, visited hash sets, BFS queue) must be completely private.
 * 2. Cyclic Graph Resilience:
 *    - Handles cycles without infinite loops or duplicate yields.
 * 3. Bounded Depth:
 *    - Traverses only up to `maxDepth` hops from the source profile.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you provide a way to access the elements of an aggregate object sequentially
 * without exposing its underlying representation?
 * ==============================================================================================
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
    std::cout << ">>> Running Social Graph BFS (Iterator) Test Suite <<<\n\n";

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

    std::cout << "\n🎉 ALL ITERATOR TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
