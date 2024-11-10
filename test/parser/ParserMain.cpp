


#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm-18/llvm/ADT/StringMap.h>
#include <llvm/ADT/StringSet.h>

#include "Common.h" // provides DEBUG_PRINT(x) macro
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"

class A{public: void print(){}};
void runLexerTests() {
    llvm::StringRef testInput1 = "call_pointcut myPointcut = func(myFunction);";
    Lexer lexer1(testInput1);
    lexer1.updateDeclarationEnd();
    ASSERT_EQ(lexer1.next().kind, TOK_CALL_POINTCUT);
    ASSERT_EQ(lexer1.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer1.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer1.next().kind, TOK_FUNC);
    ASSERT_EQ(lexer1.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer1.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer1.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer1.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer1.next().kind, TOK_EOF);

    llvm::StringRef testInput2 = "# This is a comment\nmyPointcut = func(someFunction);";
    Lexer lexer2(testInput2);
    //ASSERT_EQ(lexer2.nextToken().kind, TOK_COMMENT);
    lexer2.updateDeclarationEnd();
    ASSERT_EQ(lexer2.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer2.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer2.next().kind, TOK_FUNC);
    ASSERT_EQ(lexer2.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer2.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer2.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer2.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer2.next().kind, TOK_EOF);

    llvm::StringRef testInput3 = "myPointcut = func(func1) && func(func2);";
    Lexer lexer3(testInput3);
    lexer3.updateDeclarationEnd();
    ASSERT_EQ(lexer3.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer3.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer3.next().kind, TOK_FUNC);
    ASSERT_EQ(lexer3.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer3.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer3.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer3.next().kind, TOK_AMPAMP);
    ASSERT_EQ(lexer3.next().kind, TOK_FUNC);
    ASSERT_EQ(lexer3.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer3.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer3.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer3.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer3.next().kind, TOK_EOF);

    // clang_section = pragma_clang(bss, my_section);
    llvm::StringRef testInput4 = "clang_section = pragma_clang(bss, my_section);";
    Lexer lexer4(testInput4);
    lexer4.updateDeclarationEnd();
    ASSERT_EQ(lexer4.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer4.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer4.next().kind, TOK_PRAGMA_CLANG);
    ASSERT_EQ(lexer4.next().kind, TOK_LPAREN);
    //llvm::errs() << "Current token: " << lexer4.next().kind << "\n";
    //DEBUG_LOG("lexer4.next().kind: " << lexer4.next().kind << ", text: " << lexer4.next().text);
    ASSERT_EQ(lexer4.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer4.next().kind, TOK_COMMA);
    ASSERT_EQ(lexer4.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer4.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer4.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer4.next().kind, TOK_EOF);

    // pragma_clang || example
    llvm::StringRef testInput5 = "run_pointcut myPointcut = pragma_clang(bss, data) || pragma_clang(data, my_data);";
    Lexer lexer5(testInput5);
    lexer5.updateDeclarationEnd();
    ASSERT_EQ(lexer5.next().kind, TOK_RUN_POINTCUT);
    ASSERT_EQ(lexer5.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer5.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer5.next().kind, TOK_PRAGMA_CLANG);
    ASSERT_EQ(lexer5.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer5.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer5.next().kind, TOK_COMMA);
    ASSERT_EQ(lexer5.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer5.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer5.next().kind, TOK_PIPEPIPE);
    ASSERT_EQ(lexer5.next().kind, TOK_PRAGMA_CLANG);
    ASSERT_EQ(lexer5.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer5.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer5.next().kind, TOK_COMMA);
    ASSERT_EQ(lexer5.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer5.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer5.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer5.next().kind, TOK_EOF);

    // myPointcut = annotation_analysis(my_annotation);
    llvm::StringRef testInput6 = "myPointcut = annotation_analysis(my_annotation);";
    Lexer lexer6(testInput6);
    lexer6.updateDeclarationEnd();
    ASSERT_EQ(lexer6.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer6.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer6.next().kind, TOK_ANNOTATION_ANALYSIS);
    ASSERT_EQ(lexer6.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer6.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer6.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer6.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer6.next().kind, TOK_EOF);

    // call_pointcut myPointcut = func(...);
    llvm::StringRef testInput7 = "call_pointcut myPointcut = func(...);";
    Lexer lexer7(testInput7);
    lexer7.updateDeclarationEnd();
    ASSERT_EQ(lexer7.next().kind, TOK_CALL_POINTCUT);
    ASSERT_EQ(lexer7.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer7.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer7.next().kind, TOK_FUNC);
    ASSERT_EQ(lexer7.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer7.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer7.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer7.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer7.next().kind, TOK_EOF);

    // "call_pointcut myPointcut = func(...) && (pragma_clang(bss, my_section) || pragma_clang(data, my_data));"
    llvm::StringRef testInput8 = "call_pointcut myPointcut = func(...) && (pragma_clang(bss, my_section) || pragma_clang(data, my_data));";
    Lexer lexer8(testInput8);
    lexer8.updateDeclarationEnd();
    ASSERT_EQ(lexer8.next().kind, TOK_CALL_POINTCUT);
    ASSERT_EQ(lexer8.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer8.next().kind, TOK_EQUAL);
    ASSERT_EQ(lexer8.next().kind, TOK_FUNC);
    ASSERT_EQ(lexer8.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer8.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_AMPAMP);
    ASSERT_EQ(lexer8.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_PRAGMA_CLANG);
    ASSERT_EQ(lexer8.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer8.next().kind, TOK_COMMA);
    ASSERT_EQ(lexer8.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer8.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_PIPEPIPE);
    ASSERT_EQ(lexer8.next().kind, TOK_PRAGMA_CLANG);
    ASSERT_EQ(lexer8.next().kind, TOK_LPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer8.next().kind, TOK_COMMA);
    ASSERT_EQ(lexer8.next().kind, TOK_IDENTIFIER);
    ASSERT_EQ(lexer8.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_RPAREN);
    ASSERT_EQ(lexer8.next().kind, TOK_SEMICOLON);
    ASSERT_EQ(lexer8.next().kind, TOK_EOF);
}
void runParserTests() {
    // run_pointcut myPointcut = call(myFunction);
    llvm::StringRef testInput1 = "run_pointcut myPointcut = func(myFunction);";
    Lexer lexer1(testInput1);
    Parser parser1(lexer1);
    auto ast1 = parser1.parsePointcutList();
    ASSERT_EQ(ast1.size(), 1);
    auto *pointcut1 = dynamic_cast<PointcutDeclaration *>(ast1[0].get());
    ASSERT_NOT_NULL(pointcut1);
    ASSERT_EQ(pointcut1->isExported, true);
    ASSERT_EQ(pointcut1->name, "myPointcut");
    auto *callExpr1 = dynamic_cast<FuncExpression *>(pointcut1->expression.get());
    ASSERT_NOT_NULL(callExpr1);
    ASSERT_EQ(callExpr1->id, "myFunction");

    //myOtherPointcut = run(otherFunction);
    llvm::StringRef testInput2 = "myOtherPointcut = func(otherFunction);";
    Lexer lexer2(testInput2);
    Parser parser2(lexer2);
    auto ast2 = parser2.parsePointcutList();
    ASSERT_EQ(ast2.size(), 1);
    auto *pointcut2 = dynamic_cast<PointcutDeclaration *>(ast2[0].get());
    ASSERT_NOT_NULL(pointcut2);
    ASSERT_EQ(pointcut2->isExported, false);
    ASSERT_EQ(pointcut2->name, "myOtherPointcut");
    auto *runExpr2 = dynamic_cast<FuncExpression *>(pointcut2->expression.get());
    ASSERT_NOT_NULL(runExpr2);
    ASSERT_EQ(runExpr2->id, "otherFunction");

    // call_pointcut myPointcut = call(...);
    llvm::StringRef testInput3 = "call_pointcut myPointcut = func(...);";
    Lexer lexer3(testInput3);
    Parser parser3(lexer3);
    auto ast3 = parser3.parsePointcutList();
    ASSERT_EQ(ast3.size(), 1);
    auto *pointcut3 = dynamic_cast<PointcutDeclaration *>(ast3[0].get());
    ASSERT_NOT_NULL(pointcut3);
    ASSERT_EQ(pointcut3->isExported, true);
    ASSERT_EQ(pointcut3->name, "myPointcut");
    auto *callExpr3 = dynamic_cast<FuncExpression *>(pointcut3->expression.get());
    ASSERT_NOT_NULL(callExpr3);
    ASSERT_EQ(callExpr3->id, "...");

    // call_pointcut myPointcut = func(...) && (pragma_clang(bss, my_section) || pragma_clang(data, my_data));
    llvm::StringRef testInput4 = "run_pointcut myPointcut = func(...) && (pragma_clang(bss, my_section) || pragma_clang(data, my_data));";
    Lexer lexer4(testInput4);
    Parser parser4(lexer4);
    auto ast4 = parser4.parsePointcutList();
    ASSERT_EQ(ast4.size(), 1);
    auto *pointcut4 = dynamic_cast<PointcutDeclaration *>(ast4[0].get());
    ASSERT_NOT_NULL(pointcut4);
    ASSERT_EQ(pointcut4->isExported, true);
    ASSERT_EQ(pointcut4->name, "myPointcut");
    ASSERT_EQ(pointcut4->matchKind, MATCH_RUN);
    auto *andExpr4 = dynamic_cast<AndExpression *>(pointcut4->expression.get());
    ASSERT_NOT_NULL(andExpr4);
    auto *runExpr4 = dynamic_cast<FuncExpression *>(andExpr4->left.get());
    ASSERT_NOT_NULL(runExpr4);
    ASSERT_EQ(runExpr4->id, "...");
    auto *parExpr4 = dynamic_cast<ParenthesizedExpression *>(andExpr4->right.get());
    ASSERT_NOT_NULL(parExpr4);
    auto *orExpr4 = dynamic_cast<OrExpression *>(parExpr4->expr.get());
    ASSERT_NOT_NULL(orExpr4);
    auto *pragma1 = dynamic_cast<PragmaClangExprNode *>(orExpr4->left.get());
    ASSERT_NOT_NULL(pragma1);
    ASSERT_EQ(pragma1->pragmaKind.kind, TOK_BSS);
    ASSERT_EQ(pragma1->sectionName, "my_section");
    auto *pragma2 = dynamic_cast<PragmaClangExprNode *>(orExpr4->right.get());
    ASSERT_NOT_NULL(pragma2);
    ASSERT_EQ(pragma2->pragmaKind.kind, TOK_DATA);
    ASSERT_EQ(pragma2->sectionName, "my_data");
    

}

int main() {
    runLexerTests();
    llvm::outs() << "All lexer tests passed!\n";
    runParserTests();
    llvm::outs() << "All parser tests passed!\n";
    return 0;
}