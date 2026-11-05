/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Amazon Data Lake ETL Ingestion Pipeline (Invariant Ingestion Skeleton)
 * Category: Behavioral Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * In AWS Lake Formation / Big Data Pipelines, petabytes of raw data stream in daily from disparate
 * sources: CSV merchant financial settlements, JSON web clickstream events, and Protobuf telemetry.
 * 
 * Regardless of data format, the enterprise data compliance rules dictate an STRICT, UNALTERABLE
 * ingestion sequence:
 *   1. Connect to data source.
 *   2. Extract raw data chunks.
 *   3. Enforce schema compliance (reject corrupted records).
 *   4. Deduplicate records using idempotent key hashing (shared invariant step).
 *   5. Transform & enrich format-specific records.
 *   6. Write optimized columnar Parquet files to S3 data lake.
 *   7. Emit CloudWatch audit log (optional hook).
 * 
 * If engineers write separate ad-hoc scripts for each data format, compliance steps (like deduplication
 * or schema validation) get accidentally skipped or reordered.
 * 
 * The system must define the invariant algorithm skeleton in a base class, deferring only
 * format-specific parsing and transformation steps to subclasses without letting subclasses
 * alter the overall execution sequence.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Template Method Invariance:
 *    - The `runEtlPipeline()` template method defines the fixed algorithm order.
 * 2. Primitive Operations / Hooks:
 *    - Format-specific steps (`extractData()`, `validateSchema()`, `transformData()`) are pure virtual.
 *    - Optional hook (`shouldEmitAuditMetrics()`) with default implementation.
 * 3. Modern C++:
 *    - Non-virtual interface (NVI) idiom: `runEtlPipeline()` is public non-virtual, while
 *      primitive steps are protected virtual methods.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you define the skeleton of an algorithm in an operation, deferring some steps to subclasses,
 * letting subclasses redefine certain steps of an algorithm without changing the algorithm's structure?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Abstract Base Class (Template Method Skeleton)
// ----------------------------------------------------------------------------------------------

class DataLakeEtlPipeline {
public:
    virtual ~DataLakeEtlPipeline() = default;

    struct IngestionStats {
        size_t rawRecordsCount{0};
        size_t validRecordsCount{0};
        size_t deduplicatedCount{0};
        bool success{false};
        std::string targetS3Path;
    };

    /**
     * The Template Method.
     * Enforces the unalterable sequence of the ETL workflow.
     */
    IngestionStats runEtlPipeline(const std::string& sourceUri, const std::string& destinationBucket) {
        std::cout << ">>> Starting ETL Ingestion from: " << sourceUri << "\n";
        
        connect(sourceUri);
        auto rawData = extractRawRecords();
        auto validated = validateSchema(rawData);
        auto deduplicated = applyDeduplication(validated);
        auto transformed = transformAndEnrich(deduplicated);
        std::string s3Path = loadToDataLake(transformed, destinationBucket);

        if (shouldEmitAuditMetrics()) {
            emitAuditLog(s3Path, deduplicated.size());
        }

        return IngestionStats{
            .rawRecordsCount = rawData.size(),
            .validRecordsCount = validated.size(),
            .deduplicatedCount = deduplicated.size(),
            .success = true,
            .targetS3Path = s3Path
        };
    }

protected:
    // Invariant common steps
    void connect(const std::string& sourceUri) {
        std::cout << "  [Base] Connected to source: " << sourceUri << "\n";
    }

    std::vector<std::string> applyDeduplication(const std::vector<std::string>& records) {
        // Simple hash set deduplication
        std::vector<std::string> deduped;
        for (const auto& r : records) {
            bool found = false;
            for (const auto& existing : deduped) {
                if (existing == r) { found = true; break; }
            }
            if (!found) deduped.push_back(r);
        }
        std::cout << "  [Base] Deduplicated " << records.size() << " records down to " << deduped.size() << "\n";
        return deduped;
    }

    void emitAuditLog(const std::string& path, size_t count) {
        std::cout << "  [Audit] Logged ingestion of " << count << " records to " << path << "\n";
    }

    // Optional Hook with default implementation
    virtual bool shouldEmitAuditMetrics() const {
        return true;
    }

    // Primitive operations to be customized by subclasses (TODO: Implement in concrete classes)
    virtual std::vector<std::string> extractRawRecords() = 0;
    virtual std::vector<std::string> validateSchema(const std::vector<std::string>& raw) = 0;
    virtual std::vector<std::string> transformAndEnrich(const std::vector<std::string>& valid) = 0;
    virtual std::string loadToDataLake(const std::vector<std::string>& transformed, const std::string& destBucket) = 0;
};

// ----------------------------------------------------------------------------------------------
// Concrete Pipeline 1: CSV Sales Pipeline (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class CsvSalesDataPipeline : public DataLakeEtlPipeline {
protected:
    std::vector<std::string> extractRawRecords() override {
        // TODO: Simulate CSV row extraction (return 4 sample records, including 1 duplicate and 1 corrupted)
        return {"id=1,amt=100", "id=2,amt=200", "id=1,amt=100", "CORRUPTED_ROW"};
    }

    std::vector<std::string> validateSchema(const std::vector<std::string>& raw) override {
        // TODO: Filter out any row containing "CORRUPTED"
        (void)raw;
        return {};
    }

    std::vector<std::string> transformAndEnrich(const std::vector<std::string>& valid) override {
        // TODO: Append ",currency=USD"
        (void)valid;
        return {};
    }

    std::string loadToDataLake(const std::vector<std::string>& transformed, const std::string& destBucket) override {
        // TODO: Return destination S3 path
        (void)transformed;
        return "s3://" + destBucket + "/sales/sales_2026.parquet";
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running Data Lake ETL Ingestion (Template Method) Test Suite <<<\n\n";

    // Test Case 1: CSV Ingestion Pipeline Execution
    {
        std::cout << "[Test 1] CSV Sales Ingestion Pipeline... \n";
        CsvSalesDataPipeline pipeline;
        auto stats = pipeline.runEtlPipeline("s3://raw-bucket/sales.csv", "analytics-lake");

        assert(stats.success && "Pipeline must succeed");
        assert(stats.rawRecordsCount == 4 && "Should have 4 raw records");
        assert(stats.validRecordsCount == 3 && "Corrupted row should be filtered out");
        assert(stats.deduplicatedCount == 2 && "Duplicate row should be removed by base class");
        assert(stats.targetS3Path == "s3://analytics-lake/sales/sales_2026.parquet");
        std::cout << "PASSED\n";
    }

    std::cout << "\n🎉 ALL TEMPLATE METHOD TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
