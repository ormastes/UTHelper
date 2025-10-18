#include <string>
#include <iostream>

// A final class that should have final removed and virtual added
class DatabaseHandler final {
public:
    void connect() {
        std::cout << "Connecting to database" << std::endl;
    }

    std::string query(const std::string& sql) {
        return "Result: " + sql;
    }

private:
    void authenticate() {
        std::cout << "Authenticating" << std::endl;
    }
};

int main() {
    DatabaseHandler db;
    db.connect();
    std::string result = db.query("SELECT * FROM users");
    std::cout << result << std::endl;
    return 0;
}
