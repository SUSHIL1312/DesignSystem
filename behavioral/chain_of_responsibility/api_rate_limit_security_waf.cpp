/**
 * ==============================================================================================
 * PROBLEM STATEMENT: AWS WAF (Web Application Firewall) & API Filter Chain
 * Category: Behavioral Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * AWS WAF inspects millions of incoming web requests per second. Each request must pass through
 * a configurable chain of security filters before reaching backend services:
 *   1. `IpBlacklistFilter`: Blocks known malicious IP addresses immediately.
 *   2. `RateLimitFilter`: Rejects IPs that exceed safe per-minute velocity thresholds.
 *   3. `SqlInjectionFilter`: Scans query strings and body for SQL injection signatures (`DROP TABLE`, `UNION SELECT`).
 *   4. `AuthenticationFilter`: Validates HMAC or Bearer auth token headers.
 * 
 * Invariants:
 *   - Any filter in the chain can decide to short-circuit and terminate the request immediately
 *     (returning a security violation result).
 *   - If a filter passes, it forwards the request to the `next` filter in the chain.
 *   - If the request reaches the end of the chain without violation, it is approved.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Chain Composition:
 *    - Handlers implement `ISecurityFilter` and maintain a reference/pointer to `next_`.
 *    - Fluent chain linking (e.g. `filterA->setNext(filterB)->setNext(filterC)`).
 * 2. Short-Circuiting:
 *    - Execution stops on the first failed check without running subsequent expensive checks.
 * 3. Extensibility:
 *    - New filters (e.g., Geo-IP blocking, CSRF check) can be inserted anywhere in the chain.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * How can you avoid coupling the sender of a request to its receiver by giving more than one
 * object a chance to handle the request? Chain the receiving objects and pass the request along the chain.
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <cassert>

// ----------------------------------------------------------------------------------------------
// Domain Models
// ----------------------------------------------------------------------------------------------

struct WafRequest {
    std::string clientIp;
    std::string path;
    std::string payload;
    std::string authToken;
    int currentRps{10};
};

struct FilterResult {
    bool passed{false};
    std::string rejectingFilter;
    std::string reason;
};

// ----------------------------------------------------------------------------------------------
// Handler Interface & Base Class (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

class ISecurityFilter {
public:
    virtual ~ISecurityFilter() = default;

    virtual std::shared_ptr<ISecurityFilter> setNext(std::shared_ptr<ISecurityFilter> nextFilter) = 0;
    virtual FilterResult inspect(const WafRequest& req) = 0;
};

class BaseSecurityFilter : public ISecurityFilter {
protected:
    std::shared_ptr<ISecurityFilter> next_;

public:
    std::shared_ptr<ISecurityFilter> setNext(std::shared_ptr<ISecurityFilter> nextFilter) override {
        next_ = nextFilter;
        return next_;
    }

    FilterResult inspect(const WafRequest& req) override {
        if (next_) {
            return next_->inspect(req);
        }
        // End of chain reached successfully
        return FilterResult{.passed = true, .rejectingFilter = "", .reason = "Approved"};
    }
};

// ----------------------------------------------------------------------------------------------
// Concrete Filters (TODO: Implement inspection logic)
// ----------------------------------------------------------------------------------------------

class IpBlacklistFilter : public BaseSecurityFilter {
public:
    FilterResult inspect(const WafRequest& req) override {
        if (req.clientIp == "198.51.100.4") { // Malicious IP
            return FilterResult{
                .passed = false,
                .rejectingFilter = "IpBlacklistFilter",
                .reason = "Blocked malicious botnet IP"
            };
        }
        // TODO: Delegate to base class next handler
        return BaseSecurityFilter::inspect(req);
    }
};

class RateLimitFilter : public BaseSecurityFilter {
private:
    int maxRps_;

public:
    explicit RateLimitFilter(int maxRps) : maxRps_(maxRps) {}

    FilterResult inspect(const WafRequest& req) override {
        if (req.currentRps > maxRps_) {
            return FilterResult{
                .passed = false,
                .rejectingFilter = "RateLimitFilter",
                .reason = "RPS threshold exceeded"
            };
        }
        // TODO: Delegate to base class next handler
        return BaseSecurityFilter::inspect(req);
    }
};

class SqlInjectionFilter : public BaseSecurityFilter {
public:
    FilterResult inspect(const WafRequest& req) override {
        if (req.payload.find("UNION SELECT") != std::string::npos ||
            req.payload.find("DROP TABLE") != std::string::npos) {
            return FilterResult{
                .passed = false,
                .rejectingFilter = "SqlInjectionFilter",
                .reason = "SQL Injection signature detected"
            };
        }
        // TODO: Delegate to base class next handler
        return BaseSecurityFilter::inspect(req);
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running AWS WAF Filter (Chain of Responsibility) Test Suite <<<\n\n";

    // Setup Chain: IP Blacklist -> Rate Limit (max 100) -> SQL Injection
    auto ipFilter = std::make_shared<IpBlacklistFilter>();
    auto rateFilter = std::make_shared<RateLimitFilter>(100);
    auto sqliFilter = std::make_shared<SqlInjectionFilter>();

    ipFilter->setNext(rateFilter)->setNext(sqliFilter);

    // Test Case 1: Clean Request Passes Through Entire Chain
    {
        std::cout << "[Test 1] Valid Clean Request Approved... ";
        WafRequest req{
            .clientIp = "10.0.0.1",
            .path = "/api/v1/search",
            .payload = "query=books",
            .authToken = "valid-token",
            .currentRps = 20
        };

        FilterResult res = ipFilter->inspect(req);
        assert(res.passed && "Clean request should pass all WAF checks");
        std::cout << "PASSED\n";
    }

    // Test Case 2: Short-circuit on first filter (IP Blacklist)
    {
        std::cout << "[Test 2] Short-circuit at IP Blacklist... ";
        WafRequest badIpReq{
            .clientIp = "198.51.100.4",
            .path = "/login",
            .payload = "user=admin",
            .authToken = "none",
            .currentRps = 10
        };

        FilterResult res = ipFilter->inspect(badIpReq);
        assert(!res.passed && "Blacklisted IP must fail");
        assert(res.rejectingFilter == "IpBlacklistFilter");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Short-circuit on second filter (Rate Limit)
    {
        std::cout << "[Test 3] Short-circuit at Rate Limit Filter... ";
        WafRequest spamReq{
            .clientIp = "10.0.0.2",
            .path = "/search",
            .payload = "q=laptop",
            .authToken = "token",
            .currentRps = 250 // Exceeds 100 limit
        };

        FilterResult res = ipFilter->inspect(spamReq);
        assert(!res.passed && "Excessive RPS must fail");
        assert(res.rejectingFilter == "RateLimitFilter");
        std::cout << "PASSED\n";
    }

    // Test Case 4: Short-circuit on third filter (SQL Injection)
    {
        std::cout << "[Test 4] Short-circuit at SQL Injection Filter... ";
        WafRequest sqliReq{
            .clientIp = "10.0.0.3",
            .path = "/search",
            .payload = "id=10 UNION SELECT * FROM users",
            .authToken = "token",
            .currentRps = 15
        };

        FilterResult res = ipFilter->inspect(sqliReq);
        assert(!res.passed && "SQL injection must fail");
        assert(res.rejectingFilter == "SqlInjectionFilter");
        std::cout << "PASSED\n";
    }

    std::cout << "\n🎉 ALL CHAIN OF RESPONSIBILITY TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
