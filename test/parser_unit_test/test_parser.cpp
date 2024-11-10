#include <gtest/gtest.h>
#include <gmock/gmock.h>


// Include headers for the code under test
#include "Lexer.h"
#include "Parser.h"
#include "Token.h"
#include "Common.h"

// Include LLVM headers if necessary
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

// Helper macro for assertions (if not using GTest's ASSERT_* macros)
#define ASSERT_NOT_NULL(x) ASSERT_TRUE((x) != nullptr)

// Test fixture for the Lexer tests
class LexerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialization code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test cases for the Lexer
TEST_F(LexerTest, TestInput1) {
    llvm::StringRef testInput = "call_pointcut myPointcut = func(myFunction);";
    Lexer lexer(testInput);
    lexer.updateDeclarationEnd();

    EXPECT_EQ(lexer.next().kind, TOK_CALL_POINTCUT);
    EXPECT_EQ(lexer.next().kind, TOK_IDENTIFIER);
    EXPECT_EQ(lexer.next().kind, TOK_EQUAL);
    EXPECT_EQ(lexer.next().kind, TOK_FUNC);
    EXPECT_EQ(lexer.next().kind, TOK_LPAREN);
    EXPECT_EQ(lexer.next().kind, TOK_IDENTIFIER);
    EXPECT_EQ(lexer.next().kind, TOK_RPAREN);
    EXPECT_EQ(lexer.next().kind, TOK_SEMICOLON);
    EXPECT_EQ(lexer.next().kind, TOK_EOF);
}

// Add more tests similar to TestInput1 for other inputs...

// Test fixture for the Parser tests
class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialization code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test cases for the Parser
TEST_F(ParserTest, TestParserInput1) {
    llvm::StringRef testInput = "run_pointcut myPointcut = func(myFunction);";
    Lexer lexer(testInput);
    Parser parser(lexer);
    auto ast = parser.parsePointcutList();

    ASSERT_EQ(ast.size(), 1);
    auto *pointcut = dynamic_cast<PointcutDeclaration *>(ast[0].get());
    ASSERT_NOT_NULL(pointcut);
    EXPECT_EQ(pointcut->isExported, true);
    EXPECT_EQ(pointcut->name, "myPointcut");

    auto *callExpr = dynamic_cast<FuncExpression *>(pointcut->expression.get());
    ASSERT_NOT_NULL(callExpr);
    EXPECT_EQ(callExpr->id, "myFunction");
}

// Add more tests similar to TestParserInput1 for other inputs...

