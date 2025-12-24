/**
 * @file test_framework.hpp
 * @brief Simple test framework for EcoSim genetics tests
 * 
 * Provides macros and utilities for writing and running tests.
 */

#ifndef ECOSIM_TEST_FRAMEWORK_HPP
#define ECOSIM_TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cmath>

namespace EcoSim {
namespace Testing {

// Test result tracking
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

class TestSuite {
public:
    static TestSuite& instance() {
        static TestSuite suite;
        return suite;
    }
    
    void addResult(const std::string& name, bool passed, const std::string& msg = "") {
        results_.push_back({name, passed, msg});
        if (passed) {
            passCount_++;
        } else {
            failCount_++;
        }
    }
    
    void printSummary() const {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Test Summary: " << passCount_ << " passed, " 
                  << failCount_ << " failed" << std::endl;
        
        if (failCount_ > 0) {
            std::cout << "\nFailed tests:" << std::endl;
            for (const auto& r : results_) {
                if (!r.passed) {
                    std::cout << "  FAIL: " << r.name;
                    if (!r.message.empty()) {
                        std::cout << " - " << r.message;
                    }
                    std::cout << std::endl;
                }
            }
        }
        std::cout << "========================================\n" << std::endl;
    }
    
    bool allPassed() const { return failCount_ == 0; }
    int passCount() const { return passCount_; }
    int failCount() const { return failCount_; }
    
    void reset() {
        results_.clear();
        passCount_ = 0;
        failCount_ = 0;
    }
    
private:
    std::vector<TestResult> results_;
    int passCount_ = 0;
    int failCount_ = 0;
};

// Assertion macros
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while(0)

#define TEST_ASSERT_MSG(condition, msg) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + msg); \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            throw std::runtime_error("Expected " #expected " == " #actual); \
        } \
    } while(0)

#define TEST_ASSERT_NEAR(expected, actual, epsilon) \
    do { \
        if (std::abs((expected) - (actual)) > (epsilon)) { \
            throw std::runtime_error("Expected " #expected " near " #actual); \
        } \
    } while(0)

#define TEST_ASSERT_GT(a, b) \
    do { \
        if (!((a) > (b))) { \
            throw std::runtime_error("Expected " #a " > " #b); \
        } \
    } while(0)

#define TEST_ASSERT_GE(a, b) \
    do { \
        if (!((a) >= (b))) { \
            throw std::runtime_error("Expected " #a " >= " #b); \
        } \
    } while(0)

#define TEST_ASSERT_LT(a, b) \
    do { \
        if (!((a) < (b))) { \
            throw std::runtime_error("Expected " #a " < " #b); \
        } \
    } while(0)

#define TEST_ASSERT_LE(a, b) \
    do { \
        if (!((a) <= (b))) { \
            throw std::runtime_error("Expected " #a " <= " #b); \
        } \
    } while(0)

// Test function wrapper
#define RUN_TEST(test_func) \
    do { \
        try { \
            std::cout << "  Running " #test_func "..." << std::endl; \
            test_func(); \
            std::cout << "    PASSED" << std::endl; \
            EcoSim::Testing::TestSuite::instance().addResult(#test_func, true); \
        } catch (const std::exception& e) { \
            std::cout << "    FAILED: " << e.what() << std::endl; \
            EcoSim::Testing::TestSuite::instance().addResult(#test_func, false, e.what()); \
        } \
    } while(0)

// Test group wrapper
#define BEGIN_TEST_GROUP(name) \
    std::cout << "\n=== " << name << " ===" << std::endl;

#define END_TEST_GROUP()

} // namespace Testing
} // namespace EcoSim

#endif // ECOSIM_TEST_FRAMEWORK_HPP
