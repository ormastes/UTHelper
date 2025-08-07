// Original source file with final classes and methods
// This will be transformed by the plugin to be mockable

#include <string>
#include <iostream>

// A final class with non-virtual methods
class DatabaseConnection final {
public:
    std::string connect(const std::string& host, int port) {
        return "Connected to " + host + ":" + std::to_string(port);
    }
    
    bool executeQuery(const std::string& query) {
        std::cout << "Executing: " << query << std::endl;
        return true;
    }
    
    void disconnect() {
        std::cout << "Disconnected" << std::endl;
    }
};

// A class with final virtual methods
class Logger {
public:
    virtual void log(const std::string& message) final {
        std::cout << "[LOG] " << message << std::endl;
    }
    
    virtual int getLogLevel() final {
        return 3; // INFO level
    }
};

// A service that uses the above classes
class UserService {
private:
    DatabaseConnection* db;
    Logger* logger;
    
public:
    UserService(DatabaseConnection* database, Logger* log) 
        : db(database), logger(log) {}
    
    bool createUser(const std::string& username) {
        logger->log("Creating user: " + username);
        std::string connResult = db->connect("localhost", 5432);
        logger->log(connResult);
        
        bool result = db->executeQuery("INSERT INTO users VALUES ('" + username + "')");
        db->disconnect();
        
        return result;
    }
};