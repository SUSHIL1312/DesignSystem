/*
 * Web Application Firewall (WAF) Request Filter Chain
 *
 * Requirements:
 * - Sequential security filters linked in a chain (ISecurityFilter).
 * - Concrete filters:
 *   - IpBlacklistFilter: rejects known bad IPs.
 *   - RateLimitFilter: checks requests per second against threshold.
 *   - SqlInjectionFilter: inspects payload for SQL injection signatures.
 * - Early termination (short-circuit) on violation; forward to next filter on success.
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
    std::cout << "Running WAF filter chain tests...\n\n";

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

    std::cout << "\nAll WAF filter chain tests passed.\n";
    return 0;
}
