#pragma once

#include "Token.h"
#include <memory>
#include <string>
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"

class PointcutDeclaration;
class OrExpression;
class AndExpression;
class NotExpression;
class ParenthesizedExpression;
class FuncExpression;
class PragmaClangExprNode;
class NotationExprNode;
class NotationAnalysisExprNode;

struct ASTVisitor {
    virtual void visit(PointcutDeclaration* node) = 0;
    virtual void visit(OrExpression* node) = 0;
    virtual void visit(AndExpression* node) = 0;
    virtual void visit(NotExpression* node) = 0;
    virtual void visit(ParenthesizedExpression* node) = 0;
    virtual void visit(FuncExpression* node) = 0;
    virtual void visit(PragmaClangExprNode* node) = 0;
    virtual void visit(NotationExprNode* node) = 0;
    virtual void visit(NotationAnalysisExprNode* node) = 0;
};


enum MatchKind {
    MATCH_NONE,
    MATCH_RUN,
    MATCH_CALL,
};

class PointcutDeclaration;
using PointcutDeclarationPtr = std::unique_ptr<PointcutDeclaration>;
class ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

// AST Nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(llvm::raw_ostream &OS, int indent = 0) const = 0;
    virtual std::string getClassName() const = 0;
    virtual ASTVisitor& accept(ASTVisitor &visitor) = 0;
};

class Expression : public ASTNode {
public:
    ASTNodePtr expr;

    Expression(ASTNodePtr e);

    void print(llvm::raw_ostream &OS, int indent = 0) const override;
    // virtual ASTVisitor& accept(ASTVisitor &visitor) override;
};

class PairExpression : public ASTNode {
public:
    ASTNodePtr left;
    ASTNodePtr right;

    PairExpression(ASTNodePtr lhs, ASTNodePtr rhs);

    void print(llvm::raw_ostream &OS, int indent = 0) const override;
    // virtual ASTVisitor& accept(ASTVisitor &visitor) override;
};

class IdExpression : public ASTNode {
public:
    llvm::StringRef id;

    IdExpression(llvm::StringRef id);

    void print(llvm::raw_ostream &OS, int indent = 0) const override;
    // virtual ASTVisitor& accept(ASTVisitor &visitor) override;
};

class PointcutDeclaration : public ASTNode {
public:
    bool isExported;
    llvm::StringRef name;
    MatchKind matchKind;
    ASTNodePtr expression;

    PointcutDeclaration(bool exported, llvm::StringRef n, MatchKind matchKind, ASTNodePtr expr);

    std::string getClassName() const override;

    void print(llvm::raw_ostream &OS, int indent = 0) const override;
    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class OrExpression : public PairExpression {
public:
    OrExpression(ASTNodePtr lhs, ASTNodePtr rhs);

    std::string getClassName() const override;
    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class AndExpression : public PairExpression {
public:
    AndExpression(ASTNodePtr lhs, ASTNodePtr rhs);

    std::string getClassName() const override;
    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class NotExpression : public Expression {
public:
    NotExpression(ASTNodePtr e);

    std::string getClassName() const override;
    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class ParenthesizedExpression : public Expression {
public:
    ParenthesizedExpression(ASTNodePtr e);

    std::string getClassName() const override;
    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class FuncExpression : public IdExpression {
public:
    FuncExpression(llvm::StringRef id);

    std::string getClassName() const override;
    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class PragmaClangExprNode : public ASTNode {
public:
    Token pragmaKind;
    llvm::StringRef sectionName;

    PragmaClangExprNode(Token kind, llvm::StringRef section);

    std::string getClassName() const override;

    void print(llvm::raw_ostream &OS, int indent = 0) const override;

    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class NotationExprNode : public IdExpression {
public:
    NotationExprNode(llvm::StringRef id);

    std::string getClassName() const override;

    ASTVisitor& accept(ASTVisitor &visitor) override;
};

class NotationAnalysisExprNode : public IdExpression {
public:
    NotationAnalysisExprNode(llvm::StringRef id);

    std::string getClassName() const override;

    ASTVisitor& accept(ASTVisitor &visitor) override;
};