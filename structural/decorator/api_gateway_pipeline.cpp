/**
 * ==============================================================================================
 * PROBLEM STATEMENT: Amazon API Gateway Request/Response Transformation Pipeline
 * Category: Structural Pattern
 * Target Level: FAANG L5/L6 (Senior SWE / Staff Candidate)
 * ==============================================================================================
 * 
 * SCENARIO:
 * Amazon API Gateway / Cloudflare Edge processes millions of incoming HTTP requests per second
 * before routing them to internal microservices (AWS Lambda, ECS, DynamoDB).
 * Every API endpoint may require a dynamic, customizable set of pre- and post-processing steps:
 *   1. Core Routing: Resolves endpoint and produces raw JSON response.
 *   2. Authentication Middleware: Validates Bearer JWT tokens in "Authorization" header; returns 401 if invalid.
 *   3. Rate Limiting Middleware: Enforces per-client IP token-bucket quotas; returns 429 if exceeded.
 *   4. Observability Middleware: Generates a unique "X-Trace-Id", measures execution latency,
 *      and injects "X-Response-Time-Ms" header into the response.
 * 
 * Creating static subclasses for every combination:
 *   (e.g., AuthHandler, AuthAndRateLimitedHandler, LoggingAuthAndRateLimitedHandler...)
 * leads to a combinatorial explosion ($2^N$ classes) and violates DRY & OCP.
 * 
 * REQUIREMENTS & CONSTRAINTS:
 * 1. Single Interface Uniformity:
 *    - All decorators and core handlers must conform to the same `IHttpHandler` contract.
 *    - Callers should not know whether they are invoking a bare handler or a deeply nested pipeline.
 * 2. Dynamic Runtime Composition:
 *    - Middlewares can be chained and layered in ANY order at runtime.
 *    - A decorator can short-circuit the request (e.g., auth failure returns 401 immediately
 *      without delegating down to the inner handler).
 * 3. Modern C++ Memory Management:
 *    - Manage wrapped component ownership safely (e.g. `std::unique_ptr` or `std::shared_ptr`).
 *    - Zero memory leaks, zero dangling pointers when pipelines are dismantled.
 * 4. Extensibility:
 *    - A new middleware (e.g., Response Caching or CORS headers) must be addable without touching
 *      existing middlewares or the core handler.
 * 
 * SUBTLE ARCHITECTURAL HINT:
 * Ask yourself: How can an object wrap another object of the exact same abstract type,
 * delegating work to it while augmenting behavior before and after the call?
 * How does composition solve the inheritance explosion?
 * ==============================================================================================
 */

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>
#include <chrono>
#include <cassert>
#include <thread>
#include <sstream>

// ----------------------------------------------------------------------------------------------
// HTTP Protocol Data Models
// ----------------------------------------------------------------------------------------------

struct HttpRequest {
    std::string method;                                     // "GET", "POST", etc.
    std::string path;                                       // "/api/v1/orders"
    std::string clientIp;                                   // "192.168.1.1"
    std::unordered_map<std::string, std::string> headers;   // "Authorization", etc.
    std::string body;
};

struct HttpResponse {
    int statusCode{200};                                    // 200, 401, 429, 500
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

// ----------------------------------------------------------------------------------------------
// Skeletons & Signatures (TODO: Implement your design)
// ----------------------------------------------------------------------------------------------

/**
 * Component Interface: Common contract for both concrete handlers and decorators.
 */
class IHttpHandler {
public:
    virtual ~IHttpHandler() = default;

    // TODO: Define the primary execution contract
    virtual HttpResponse handle(const HttpRequest& request) = 0;
};

/**
 * Concrete Component: Base API endpoint router (simulating backend execution).
 */
class BaseHttpHandler : public IHttpHandler {
public:
    HttpResponse handle(const HttpRequest& request) override {
        // Simulates executing the underlying microservice business logic
        HttpResponse response;
        response.statusCode = 200;
        response.headers["Content-Type"] = "application/json";
        response.body = "{\"status\":\"success\",\"path\":\"" + request.path + "\"}";
        return response;
    }
};

/**
 * Base Decorator: Holds a reference/pointer to a wrapped IHttpHandler and delegates execution.
 */
class HttpHandlerDecorator : public IHttpHandler {
protected:
    std::unique_ptr<IHttpHandler> wrappedHandler_;

public:
    explicit HttpHandlerDecorator(std::unique_ptr<IHttpHandler> handler)
        : wrappedHandler_(std::move(handler)) {}

    // TODO: Implement default delegation
    HttpResponse handle(const HttpRequest& request) override {
        if (wrappedHandler_) {
            return wrappedHandler_->handle(request);
        }
        return HttpResponse{.statusCode = 500, .body = "Internal Server Error: No handler wrapped"};
    }
};

/**
 * Concrete Decorator 1: Authentication Middleware
 * Requirements:
 * - Checks if header "Authorization" exists and equals "Bearer valid-faang-token".
 * - If valid, delegates to `wrappedHandler_`.
 * - If invalid or missing, short-circuits and immediately returns HTTP 401 with body:
 *   "{\"error\":\"Unauthorized\"}".
 */
class AuthenticationMiddleware : public HttpHandlerDecorator {
public:
    explicit AuthenticationMiddleware(std::unique_ptr<IHttpHandler> handler)
        : HttpHandlerDecorator(std::move(handler)) {}

    HttpResponse handle(const HttpRequest& request) override {
        // TODO: Implement authentication check and short-circuit logic
        (void)request;
        return HttpHandlerDecorator::handle(request);
    }
};

/**
 * Concrete Decorator 2: Rate Limiting Middleware
 * Requirements:
 * - Accepts a maxRequestsPerClient limit in constructor.
 * - Tracks request count per `request.clientIp`.
 * - If client exceeds limit, short-circuits and returns HTTP 429 with body:
 *   "{\"error\":\"Too Many Requests\"}".
 * - Otherwise increments count and delegates to `wrappedHandler_`.
 */
class RateLimitingMiddleware : public HttpHandlerDecorator {
private:
    [[maybe_unused]] int maxRequests_;
    std::unordered_map<std::string, int> clientRequestCounts_;

public:
    RateLimitingMiddleware(std::unique_ptr<IHttpHandler> handler, int maxRequests)
        : HttpHandlerDecorator(std::move(handler)), maxRequests_(maxRequests) {}

    HttpResponse handle(const HttpRequest& request) override {
        // TODO: Implement rate limiting verification and short-circuit logic
        (void)request;
        return HttpHandlerDecorator::handle(request);
    }
};

/**
 * Concrete Decorator 3: Observability (Logging & Tracing) Middleware
 * Requirements:
 * - Generates an "X-Trace-Id" (e.g., "trace-1001", "trace-1002" or UUID).
 * - Records execution time around the `wrappedHandler_->handle()` invocation.
 * - Injects "X-Trace-Id" and "X-Response-Time-Ms" into the response headers.
 */
class ObservabilityMiddleware : public HttpHandlerDecorator {
private:
    static inline uint64_t traceCounter_{1000};

public:
    explicit ObservabilityMiddleware(std::unique_ptr<IHttpHandler> handler)
        : HttpHandlerDecorator(std::move(handler)) {}

    HttpResponse handle(const HttpRequest& request) override {
        // TODO: Implement trace generation, latency measurement, and header injection
        (void)request;
        return HttpHandlerDecorator::handle(request);
    }
};

// ----------------------------------------------------------------------------------------------
// Test Harness (Pre-written test cases to validate your design)
// ----------------------------------------------------------------------------------------------

int main() {
    std::cout << ">>> Running API Gateway Pipeline Test Suite <<<\n\n";

    // Test Case 1: Bare Handler (No middleware attached)
    {
        std::cout << "[Test 1] Bare BaseHttpHandler Execution... ";
        auto bareHandler = std::make_unique<BaseHttpHandler>();
        HttpRequest req{.method = "GET", .path = "/api/v1/inventory", .clientIp = "10.0.0.1"};

        HttpResponse res = bareHandler->handle(req);
        assert(res.statusCode == 200);
        assert(res.body.find("/api/v1/inventory") != std::string::npos);
        std::cout << "PASSED\n";
    }

    // Test Case 2: Authentication Middleware Enforcement
    {
        std::cout << "[Test 2] Authentication Middleware (Unauthorized & Authorized)... ";
        auto pipeline = std::make_unique<AuthenticationMiddleware>(
            std::make_unique<BaseHttpHandler>()
        );

        // 2a: Missing auth token
        HttpRequest unauthReq{.method = "POST", .path = "/api/v1/checkout", .clientIp = "10.0.0.2"};
        HttpResponse unauthRes = pipeline->handle(unauthReq);
        assert(unauthRes.statusCode == 401 && "Should return 401 when Authorization header is missing");
        assert(unauthRes.body.find("Unauthorized") != std::string::npos);

        // 2b: Valid auth token
        HttpRequest authReq{.method = "POST", .path = "/api/v1/checkout", .clientIp = "10.0.0.2"};
        authReq.headers["Authorization"] = "Bearer valid-faang-token";
        HttpResponse authRes = pipeline->handle(authReq);
        assert(authRes.statusCode == 200 && "Should return 200 with valid Bearer token");
        std::cout << "PASSED\n";
    }

    // Test Case 3: Rate Limiting Middleware Quota Enforcement
    {
        std::cout << "[Test 3] Rate Limiting Middleware (Quota Exceeded)... ";
        constexpr int limit = 3;
        auto pipeline = std::make_unique<RateLimitingMiddleware>(
            std::make_unique<BaseHttpHandler>(),
            limit
        );

        HttpRequest req{.method = "GET", .path = "/api/v1/search", .clientIp = "172.16.0.5"};

        for (int i = 1; i <= limit; ++i) {
            HttpResponse res = pipeline->handle(req);
            assert(res.statusCode == 200 && "Requests within quota should succeed");
        }

        // 4th request must be rejected
        HttpResponse rejectedRes = pipeline->handle(req);
        assert(rejectedRes.statusCode == 429 && "Exceeding rate limit must yield 429");
        assert(rejectedRes.body.find("Too Many Requests") != std::string::npos);
        std::cout << "PASSED\n";
    }

    // Test Case 4: Full Multi-layered Middleware Pipeline Composition
    // Pipeline Composition: Observability -> RateLimiting (limit=2) -> Authentication -> BaseHandler
    {
        std::cout << "[Test 4] Full Decorator Stack Composition... ";
        
        auto fullPipeline = std::make_unique<ObservabilityMiddleware>(
            std::make_unique<RateLimitingMiddleware>(
                std::make_unique<AuthenticationMiddleware>(
                    std::make_unique<BaseHttpHandler>()
                ),
                2 // limit
            )
        );

        HttpRequest req{.method = "GET", .path = "/api/v1/profile", .clientIp = "192.168.1.50"};
        req.headers["Authorization"] = "Bearer valid-faang-token";

        // Request 1: Valid & within rate limit
        HttpResponse res1 = fullPipeline->handle(req);
        assert(res1.statusCode == 200);
        assert(res1.headers.count("X-Trace-Id") > 0 && "Response must contain X-Trace-Id header");
        assert(res1.headers.count("X-Response-Time-Ms") > 0 && "Response must contain X-Response-Time-Ms header");

        // Request 2: Valid
        HttpResponse res2 = fullPipeline->handle(req);
        assert(res2.statusCode == 200);

        // Request 3: Hits Rate Limit (429), BUT still has Observability headers because Observability is outer!
        HttpResponse res3 = fullPipeline->handle(req);
        assert(res3.statusCode == 429);
        assert(res3.headers.count("X-Trace-Id") > 0 && "Outer observability middleware must trace even rejected requests");
        std::cout << "PASSED\n";
    }

    // Test Case 5: Dynamic Extension - New Custom Gzip Middleware (OCP Validation)
    {
        std::cout << "[Test 5] Dynamic Extension (Custom Response Header Middleware)... ";

        class SecurityHeadersMiddleware : public HttpHandlerDecorator {
        public:
            explicit SecurityHeadersMiddleware(std::unique_ptr<IHttpHandler> handler)
                : HttpHandlerDecorator(std::move(handler)) {}

            HttpResponse handle(const HttpRequest& request) override {
                HttpResponse res = HttpHandlerDecorator::handle(request);
                res.headers["X-Content-Type-Options"] = "nosniff";
                res.headers["X-Frame-Options"] = "DENY";
                return res;
            }
        };

        auto customPipeline = std::make_unique<SecurityHeadersMiddleware>(
            std::make_unique<BaseHttpHandler>()
        );

        HttpRequest req{.method = "GET", .path = "/health", .clientIp = "127.0.0.1"};
        HttpResponse res = customPipeline->handle(req);
        assert(res.statusCode == 200);
        assert(res.headers["X-Content-Type-Options"] == "nosniff");
        assert(res.headers["X-Frame-Options"] == "DENY");
        std::cout << "PASSED\n";
    }

    std::cout << "\n🎉 ALL API GATEWAY PIPELINE TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
