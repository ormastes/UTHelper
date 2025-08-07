// Google Mock test for the transformed code
// This test will only compile if the plugin successfully removes final and makes methods virtual

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Include the transformed version of the classes
// In a real scenario, this would be the output of the plugin
#include "test_mock_final_transformed.h"

using ::testing::Return;
using ::testing::_;

// Mock for DatabaseConnection (was final, now mockable)
class MockDatabaseConnection : public DatabaseConnection {
public:
    MOCK_METHOD(std::string, connect, (const std::string& host, int port), (override));
    MOCK_METHOD(bool, executeQuery, (const std::string& query), (override));
    MOCK_METHOD(void, disconnect, (), (override));
};

// Mock for Logger (had final methods, now mockable)
class MockLogger : public Logger {
public:
    MOCK_METHOD(void, log, (const std::string& message), (override));
    MOCK_METHOD(int, getLogLevel, (), (override));
};

// Test fixture
class UserServiceTest : public ::testing::Test {
protected:
    MockDatabaseConnection* mockDb;
    MockLogger* mockLogger;
    UserService* userService;
    
    void SetUp() override {
        mockDb = new MockDatabaseConnection();
        mockLogger = new MockLogger();
        userService = new UserService(mockDb, mockLogger);
    }
    
    void TearDown() override {
        delete userService;
        delete mockLogger;
        delete mockDb;
    }
};

TEST_F(UserServiceTest, CreateUserSuccess) {
    // Set up expectations
    EXPECT_CALL(*mockLogger, log("Creating user: testuser"))
        .Times(1);
    
    EXPECT_CALL(*mockDb, connect("localhost", 5432))
        .WillOnce(Return("Connected to localhost:5432"));
    
    EXPECT_CALL(*mockLogger, log("Connected to localhost:5432"))
        .Times(1);
    
    EXPECT_CALL(*mockDb, executeQuery("INSERT INTO users VALUES ('testuser')"))
        .WillOnce(Return(true));
    
    EXPECT_CALL(*mockDb, disconnect())
        .Times(1);
    
    // Execute the test
    bool result = userService->createUser("testuser");
    
    // Verify the result
    EXPECT_TRUE(result);
}

TEST_F(UserServiceTest, CreateUserFailure) {
    // Set up expectations for a failure scenario
    EXPECT_CALL(*mockLogger, log(_))
        .Times(2); // Called twice: once for "Creating user" and once for connection
    
    EXPECT_CALL(*mockDb, connect(_, _))
        .WillOnce(Return("Connected"));
    
    EXPECT_CALL(*mockDb, executeQuery(_))
        .WillOnce(Return(false)); // Simulate query failure
    
    EXPECT_CALL(*mockDb, disconnect())
        .Times(1);
    
    // Execute the test
    bool result = userService->createUser("invalid_user");
    
    // Verify the result
    EXPECT_FALSE(result);
}

TEST_F(UserServiceTest, LoggerLogLevel) {
    // Test that we can mock the logger's getLogLevel method
    EXPECT_CALL(*mockLogger, getLogLevel())
        .WillOnce(Return(5)); // DEBUG level
    
    int level = mockLogger->getLogLevel();
    EXPECT_EQ(level, 5);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}