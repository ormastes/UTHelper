#include "ASTNode.h"

#include "ASTNode.h"
#include <utility>

using namespace std;

Expression::Expression(ASTNodePtr e)
    : expr(std::move(e)) {}

void Expression::print(llvm::raw_ostream &OS, int indent) const {
    OS.indent(indent) << getClassName() << ":\n";
    expr->print(OS, indent + 2);
}

PairExpression::PairExpression(ASTNodePtr lhs, ASTNodePtr rhs)
    : left(std::move(lhs)), right(std::move(rhs)) {}

void PairExpression::print(llvm::raw_ostream &OS, int indent) const {
    OS.indent(indent) << getClassName() << ":\n";
    left->print(OS, indent + 2);
    right->print(OS, indent + 2);
}

IdExpression::IdExpression(llvm::StringRef id)
    : id(id) {}

void IdExpression::print(llvm::raw_ostream &OS, int indent) const {
    OS.indent(indent) << getClassName() << ": " << id << "\n";
}

PointcutDeclaration::PointcutDeclaration(bool exported, llvm::StringRef n, MatchKind matchKind, ASTNodePtr expr)
    : isExported(exported), name(n), matchKind(matchKind), expression(std::move(expr)) {}

std::string PointcutDeclaration::getClassName() const {
    return "PointcutDeclaration";
}

void PointcutDeclaration::print(llvm::raw_ostream &OS, int indent) const {
    OS.indent(indent) << (isExported ? "export " : "") << getClassName() << ": " << name << "\n";
    if (expression) {
        expression->print(OS, indent + 2);
    }
}

ASTVisitor& PointcutDeclaration::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

OrExpression::OrExpression(ASTNodePtr lhs, ASTNodePtr rhs)
    : PairExpression(std::move(lhs), std::move(rhs)) {}

std::string OrExpression::getClassName() const {
    return "OrExpression";
}

ASTVisitor& OrExpression::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

AndExpression::AndExpression(ASTNodePtr lhs, ASTNodePtr rhs)
    : PairExpression(std::move(lhs), std::move(rhs)) {}

std::string AndExpression::getClassName() const {
    return "AndExpression";
}

ASTVisitor& AndExpression::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

NotExpression::NotExpression(ASTNodePtr e)
    : Expression(std::move(e)) {}

std::string NotExpression::getClassName() const {
    return "NotExpression";
}

ASTVisitor& NotExpression::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

ParenthesizedExpression::ParenthesizedExpression(ASTNodePtr e)
    : Expression(std::move(e)) {}

std::string ParenthesizedExpression::getClassName() const {
    return "ParenthesizedExpression";
}

ASTVisitor& ParenthesizedExpression::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

FuncExpression::FuncExpression(llvm::StringRef id)
    : IdExpression(id) {}

std::string FuncExpression::getClassName() const {
    return "FuncExpression";
}

ASTVisitor& FuncExpression::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

PragmaClangExprNode::PragmaClangExprNode(Token kind, llvm::StringRef section)
    : pragmaKind(kind), sectionName(section) {}

std::string PragmaClangExprNode::getClassName() const {
    return "PragmaClangExprNode";
}

void PragmaClangExprNode::print(llvm::raw_ostream &OS, int indent) const {
    OS.indent(indent) << getClassName() << ": " << pragmaKind << ", " << sectionName << "\n";
}

ASTVisitor& PragmaClangExprNode::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

NotationExprNode::NotationExprNode(llvm::StringRef id)
    : IdExpression(id) {}

std::string NotationExprNode::getClassName() const {
    return "NotationExprNode";
}

ASTVisitor& NotationExprNode::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}

NotationAnalysisExprNode::NotationAnalysisExprNode(llvm::StringRef id)
    : IdExpression(id) {}

std::string NotationAnalysisExprNode::getClassName() const {
    return "NotationAnalysisExprNode";
}

ASTVisitor& NotationAnalysisExprNode::accept(ASTVisitor &visitor) {
    visitor.visit(this);
    return visitor;
}