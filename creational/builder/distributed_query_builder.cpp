/*
 * Distributed SQL Query Builder
 *
 * Requirements:
 * - Provide a fluent API for constructing analytical SQL queries step-by-step
 *   (select, from, where, groupBy, orderBy, limit, timeout).
 * - Invariant: building a query without a target table (from) must throw std::logic_error.
 * - The built SqlQuery product must be immutable once created.
 * - Include director methods for generating standard pre-configured query templates.
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
#include <stdexcept>
#include <sstream>

// ----------------------------------------------------------------------------------------------
// Product: Immutable SQL Query Object
// ----------------------------------------------------------------------------------------------

class SqlQuery {
    friend class SqlQueryBuilder;
private:
    std::string table_;
    std::vector<std::string> projections_;
    std::vector<std::string> predicates_;
    std::vector<std::string> groupBys_;
    std::string orderBy_;
    [[maybe_unused]] bool isAscending_{true};
    int limit_{-1};
    int timeoutMs_{30000};

    // Private constructor: Only SqlQueryBuilder can construct instances
    SqlQuery() = default;

public:
    std::string toSqlString() const {
        // TODO: Assemble standard SQL string:
        // SELECT <cols> FROM <table> WHERE <cond1 AND cond2> GROUP BY <cols> ORDER BY <col> [ASC|DESC] LIMIT <n>
        return "";
    }

    std::string_view getTable() const noexcept { return table_; }
    const std::vector<std::string>& getProjections() const noexcept { return projections_; }
    const std::vector<std::string>& getPredicates() const noexcept { return predicates_; }
    int getLimit() const noexcept { return limit_; }
    int getTimeoutMs() const noexcept { return timeoutMs_; }
};

// ----------------------------------------------------------------------------------------------
// Builder Interface & Concrete Builder (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class ISqlQueryBuilder {
public:
    virtual ~ISqlQueryBuilder() = default;

    virtual ISqlQueryBuilder& from(std::string table) = 0;
    virtual ISqlQueryBuilder& select(std::string column) = 0;
    virtual ISqlQueryBuilder& where(std::string condition) = 0;
    virtual ISqlQueryBuilder& groupBy(std::string column) = 0;
    virtual ISqlQueryBuilder& orderBy(std::string column, bool ascending = true) = 0;
    virtual ISqlQueryBuilder& limit(int maxRows) = 0;
    virtual ISqlQueryBuilder& timeout(int timeoutMs) = 0;

    virtual std::unique_ptr<SqlQuery> build() = 0;
    virtual void reset() = 0;
};

class SqlQueryBuilder : public ISqlQueryBuilder {
private:
    std::unique_ptr<SqlQuery> query_;

public:
    SqlQueryBuilder() {
        reset();
    }

    void reset() override {
        // TODO: Reinitialize internal query product
        query_.reset(new SqlQuery());
    }

    ISqlQueryBuilder& from([[maybe_unused]] std::string table) override {
        // TODO: Implement
        return *this;
    }

    ISqlQueryBuilder& select([[maybe_unused]] std::string column) override {
        // TODO: Implement
        return *this;
    }

    ISqlQueryBuilder& where([[maybe_unused]] std::string condition) override {
        // TODO: Implement
        return *this;
    }

    ISqlQueryBuilder& groupBy([[maybe_unused]] std::string column) override {
        // TODO: Implement
        return *this;
    }

    ISqlQueryBuilder& orderBy([[maybe_unused]] std::string column, [[maybe_unused]] bool ascending = true) override {
        // TODO: Implement
        return *this;
    }

    ISqlQueryBuilder& limit([[maybe_unused]] int maxRows) override {
        // TODO: Implement with validation
        return *this;
    }

    ISqlQueryBuilder& timeout([[maybe_unused]] int timeoutMs) override {
        // TODO: Implement
        return *this;
    }

    std::unique_ptr<SqlQuery> build() override {
        // TODO: Validate that target table is specified; throw std::logic_error if missing
        return nullptr;
    }
};

/**
 * Director: Orchestrates standardized query templates using the builder interface.
 */
class AnalyticsQueryDirector {
public:
    static std::unique_ptr<SqlQuery> buildDailyUserActivityReport(ISqlQueryBuilder& builder, const std::string& date) {
        builder.reset();
        // TODO: Direct builder to construct:
        // SELECT user_id, COUNT(event_id) FROM user_events WHERE event_date = '<date>' GROUP BY user_id LIMIT 100
        (void)date;
        return builder.build();
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << "Running distributed query builder tests...\n\n";

    // Test Case 1: Standard Fluent Query Construction
    {
        std::cout << "[Test 1] Fluent SQL Query Construction... ";
        SqlQueryBuilder builder;
        auto query = builder.from("orders")
                            .select("order_id")
                            .select("customer_id")
                            .select("total_amount")
                            .where("status = 'COMPLETED'")
                            .where("total_amount > 100")
                            .orderBy("total_amount", false)
                            .limit(50)
                            .timeout(5000)
                            .build();

        assert(query != nullptr && "Query must be built successfully");
        assert(query->getTable() == "orders");
        assert(query->getProjections().size() == 3);
        assert(query->getPredicates().size() == 2);
        assert(query->getLimit() == 50);
        assert(query->getTimeoutMs() == 5000);

        std::string sql = query->toSqlString();
        assert(sql.find("SELECT order_id, customer_id, total_amount") != std::string::npos);
        assert(sql.find("FROM orders") != std::string::npos);
        assert(sql.find("WHERE status = 'COMPLETED' AND total_amount > 100") != std::string::npos);
        assert(sql.find("ORDER BY total_amount DESC") != std::string::npos);
        assert(sql.find("LIMIT 50") != std::string::npos);
        std::cout << "PASSED\n";
    }

    // Test Case 2: Validation Failure When Table Missing
    {
        std::cout << "[Test 2] Invariant Validation (Missing Table)... ";
        SqlQueryBuilder builder;
        bool caught = false;
        try {
            // Attempting to build without calling .from()
            builder.select("count(*)").where("x > 0").build();
        } catch (const std::logic_error&) {
            caught = true;
        }
        assert(caught && "Building query without 'from' table must throw std::logic_error");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Director Automated Template
    {
        std::cout << "[Test 3] Director Template Query Creation... ";
        SqlQueryBuilder builder;
        auto reportQuery = AnalyticsQueryDirector::buildDailyUserActivityReport(builder, "2026-11-06");

        assert(reportQuery != nullptr);
        assert(reportQuery->getTable() == "user_events");
        assert(reportQuery->getLimit() == 100);
        std::cout << "PASSED\n";
    }

    std::cout << "\nAll distributed query builder tests passed.\n";
    return 0;
}
