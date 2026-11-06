/*
 * Hierarchical Storage Tree (Files & Folders)
 *
 * Requirements:
 * - Unified IFileSystemNode interface for files (leaves) and folders (composites).
 * - Recursive storage calculation (getSizeInBytes) aggregating total bytes across arbitrarily
 *   nested folder hierarchies.
 * - Safety check: invoking composite operations (add, remove) on leaf nodes must throw std::logic_error.
 *
 * Practice Reference:
 * You can refer to https://systemdesignmastry.com/ for practicing interview-asked HLD & LLD questions.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cassert>
#include <numeric>
#include <stdexcept>

// ----------------------------------------------------------------------------------------------
// Component Interface (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class IFileSystemNode {
public:
    virtual ~IFileSystemNode() = default;

    virtual std::string_view getName() const noexcept = 0;
    virtual uint64_t getSizeInBytes() const = 0;
    virtual bool isDirectory() const noexcept = 0;

    // Composite operations (safety vs transparency)
    virtual void add([[maybe_unused]] std::shared_ptr<IFileSystemNode> child) {
        throw std::logic_error("Operation not supported on leaf node");
    }
    virtual void remove([[maybe_unused]] std::shared_ptr<IFileSystemNode> child) {
        throw std::logic_error("Operation not supported on leaf node");
    }
    virtual const std::vector<std::shared_ptr<IFileSystemNode>>& getChildren() const {
        static const std::vector<std::shared_ptr<IFileSystemNode>> empty;
        return empty;
    }
};

// ----------------------------------------------------------------------------------------------
// Leaf Component: File
// ----------------------------------------------------------------------------------------------

class DriveFile : public IFileSystemNode {
private:
    std::string name_;
    uint64_t sizeBytes_{0};

public:
    DriveFile(std::string name, uint64_t sizeBytes)
        : name_(std::move(name)), sizeBytes_(sizeBytes) {}

    std::string_view getName() const noexcept override { return name_; }
    uint64_t getSizeInBytes() const override { return sizeBytes_; }
    bool isDirectory() const noexcept override { return false; }
};

// ----------------------------------------------------------------------------------------------
// Composite Component: Folder / Directory (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class DriveFolder : public IFileSystemNode {
private:
    std::string name_;
    std::vector<std::shared_ptr<IFileSystemNode>> children_;

public:
    explicit DriveFolder(std::string name) : name_(std::move(name)) {}

    std::string_view getName() const noexcept override { return name_; }
    bool isDirectory() const noexcept override { return true; }

    void add(std::shared_ptr<IFileSystemNode> child) override {
        // TODO: Append child node to children_
        (void)child;
    }

    void remove(std::shared_ptr<IFileSystemNode> child) override {
        // TODO: Remove child from children_
        (void)child;
    }

    const std::vector<std::shared_ptr<IFileSystemNode>>& getChildren() const override {
        return children_;
    }

    uint64_t getSizeInBytes() const override {
        // TODO: Calculate recursive sum of all child nodes' sizes
        return 0;
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running filesystem composite tests...\n\n";

    // Test Case 1: Individual Leaf File Size
    {
        std::cout << "[Test 1] Leaf File Storage Calculation... ";
        auto file = std::make_shared<DriveFile>("resume.pdf", 1024 * 500); // 500 KB
        assert(file->getSizeInBytes() == 512000);
        assert(!file->isDirectory());
        std::cout << "PASSED\n";
    }

    // Test Case 2: Hierarchical Folder Aggregation
    // Structure:
    // RootFolder (total expected = 500KB + 2MB + 10MB + 5MB = 17.5 MB = 18,350,080 bytes)
    //   ├── resume.pdf (500 KB)
    //   ├── profile.png (2 MB)
    //   └── Projects/
    //         ├── source_code.zip (10 MB)
    //         └── Datasets/
    //               └── training_data.csv (5 MB)
    {
        std::cout << "[Test 2] Recursive Hierarchical Storage Aggregation... ";
        auto root = std::make_shared<DriveFolder>("My Drive");
        auto f1 = std::make_shared<DriveFile>("resume.pdf", 500 * 1024);
        auto f2 = std::make_shared<DriveFile>("profile.png", 2 * 1024 * 1024);

        auto projects = std::make_shared<DriveFolder>("Projects");
        auto f3 = std::make_shared<DriveFile>("source_code.zip", 10 * 1024 * 1024);

        auto datasets = std::make_shared<DriveFolder>("Datasets");
        auto f4 = std::make_shared<DriveFile>("training_data.csv", 5 * 1024 * 1024);

        datasets->add(f4);
        projects->add(f3);
        projects->add(datasets);

        root->add(f1);
        root->add(f2);
        root->add(projects);

        uint64_t expectedProjectsSize = (10 * 1024 * 1024) + (5 * 1024 * 1024);
        assert(projects->getSizeInBytes() == expectedProjectsSize && "Projects folder must equal 15 MB");

        uint64_t expectedRootSize = (500 * 1024) + (2 * 1024 * 1024) + expectedProjectsSize;
        assert(root->getSizeInBytes() == expectedRootSize && "Root folder must aggregate entire tree size");

        std::cout << "PASSED (Total Root: " << root->getSizeInBytes() << " bytes)\n";
    }

    // Test Case 3: Safety Check - Calling add on leaf file must throw logic_error
    {
        std::cout << "[Test 3] Composite Safety Check on Leaf Nodes... ";
        auto file = std::make_shared<DriveFile>("notes.txt", 100);
        auto child = std::make_shared<DriveFile>("sub.txt", 50);

        bool caught = false;
        try {
            file->add(child);
        } catch (const std::logic_error&) {
            caught = true;
        }
        assert(caught && "Adding child to leaf file must throw std::logic_error");
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll filesystem composite tests passed.\n";
    return 0;
}
