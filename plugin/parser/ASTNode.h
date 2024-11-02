#pragma once

#include "Token.h"
#include <memory>

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
    virtual string getClassName() const = 0;
    virtual ASTVisitor& accept(ASTVisitor &visitor) =0;
};


class Expression : public ASTNode {
public:
    ASTNodePtr expr;

    Expression(ASTNodePtr e)
        : expr(std::move(e)) {}

    void print(llvm::raw_ostream &OS, int indent = 0) const override {
        OS.indent(indent) << getClassName() << ":\n";
        expr->print(OS, indent + 2);
    }
    //virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class PairExpression : public ASTNode {
public:
    ASTNodePtr left;
    ASTNodePtr right;

    PairExpression(ASTNodePtr lhs, ASTNodePtr rhs)
        : left(std::move(lhs)), right(std::move(rhs)) {}


    void print(llvm::raw_ostream &OS, int indent = 0) const override {
        OS.indent(indent) << getClassName() << ":\n";
        left->print(OS, indent + 2);
        right->print(OS, indent + 2);
    }
    //virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class IdExpression : public ASTNode {
public:
    llvm::StringRef id;

    IdExpression(llvm::StringRef id) : id(id) {}

    void print(llvm::raw_ostream &OS, int indent = 0) const override {
        OS.indent(indent) << getClassName() <<": " << id << "\n";
    }
    //virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class PointcutDeclaration : public ASTNode {
public:
    bool isExported;
    llvm::StringRef name;
    MatchKind matchKind;
    ASTNodePtr expression;

    PointcutDeclaration(bool exported, llvm::StringRef n, MatchKind matchKind, ASTNodePtr expr)
        : isExported(exported), name(n), matchKind(matchKind), expression(std::move(expr)) {}

    virtual string getClassName() const override {
        return "PointcutDeclaration";
    }

    void print(llvm::raw_ostream &OS, int indent = 0) const override {
        OS.indent(indent) << (isExported ? "export " : "") << getClassName() << ": " << name << "\n";
        if (expression) {
            expression->print(OS, indent + 2);
        }
    }
    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};


class OrExpression : public PairExpression {
public:
    OrExpression(ASTNodePtr lhs, ASTNodePtr rhs)
        : PairExpression(std::move(lhs), std::move(rhs)) {}
    virtual string getClassName() const override {
        return "OrExpression";
    }
    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class AndExpression : public PairExpression {
public:
    AndExpression(ASTNodePtr lhs, ASTNodePtr rhs)
        : PairExpression(std::move(lhs), std::move(rhs)) {}
    virtual string getClassName() const override {
        return "AndExpression";
    }
    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class NotExpression : public Expression {
public:
    NotExpression(ASTNodePtr e)
        : Expression(std::move(e)) {}
    virtual string getClassName() const override {
        return "NotExpression";
    }
    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class ParenthesizedExpression : public Expression {
public:
    ParenthesizedExpression(ASTNodePtr e)
        : Expression(std::move(e)) {}
    virtual string getClassName() const override {
        return "ParenthesizedExpression";
    }
    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class FuncExpression : public IdExpression {
public:
    FuncExpression(llvm::StringRef id)
        : IdExpression(id) {}
    virtual string getClassName() const override {
        return "FuncExpression";
    }
    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class PragmaClangExprNode : public ASTNode {
public:
    Token pragmaKind;
    llvm::StringRef sectionName;

    PragmaClangExprNode(Token kind, llvm::StringRef section)
        : pragmaKind(kind), sectionName(section) {}

    virtual string getClassName() const override {
        return "PragmaClangExprNode";
    }

    void print(llvm::raw_ostream &OS, int indent = 0) const override {
        OS.indent(indent) << getClassName() << ": " << pragmaKind << ", " << sectionName << "\n";
    }

    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class NotationExprNode : public IdExpression {
public:
    NotationExprNode(llvm::StringRef id)
        : IdExpression(std::move(id)) {}
    virtual string getClassName() const override {
        return "NotationExprNode";
    }

    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};

class NotationAnalysisExprNode : public IdExpression {
public:
    NotationAnalysisExprNode(llvm::StringRef id)
        : IdExpression(std::move(id)) {}
    virtual string getClassName() const override {
        return "NotationAnalysisExprNode";
    }

    virtual ASTVisitor& accept(ASTVisitor &visitor) override {visitor.visit(this); return visitor;}
};