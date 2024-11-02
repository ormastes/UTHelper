#include "ASTMakeMatcherVisitor.h"
#include "AST2Matcher.h"
#include "ASTNode.h"


AbstractAST2MatcherPtr ASTMakeMatcherVisitor::getMatcher() {
    return std::move(matcher);
}

void ASTMakeMatcherVisitor::visit(PointcutDeclaration* node) {
    node->expression->accept(*this);
    auto expr = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    if (node->matchKind == MATCH_RUN) {
        matcher = std::make_unique<RunMatcher>(node, std::move(expr));
    } else if(node->matchKind == MATCH_CALL) {
        matcher = std::make_unique<CallMatcher>(node, std::move(expr));
    } else {
        assert(false);
    }
}

void ASTMakeMatcherVisitor::visit(OrExpression* node) {
    node->left->accept(*this);
    auto lhs = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    node->right->accept(*this);
    auto rhs = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    matcher = std::make_unique<OrMatcher>(node, std::move(lhs), std::move(rhs));
}

void ASTMakeMatcherVisitor::visit(AndExpression* node) {
    node->left->accept(*this);
    auto lhs = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    node->right->accept(*this);
    auto rhs = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    matcher = std::make_unique<AndMatcher>(node, std::move(lhs), std::move(rhs));
}

void ASTMakeMatcherVisitor::visit(NotExpression* node) {
    node->expr->accept(*this);
    auto inner = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    matcher = std::make_unique<NotMatcher>(node, std::move(inner));
}

void ASTMakeMatcherVisitor::visit(ParenthesizedExpression* node) {
    node->expr->accept(*this);
    auto inner = std::unique_ptr<DeclMatcher>(static_cast<DeclMatcher*>(matcher.release()));
    matcher = std::make_unique<ParenthesizedMatcher>(node, std::move(inner));
}

void ASTMakeMatcherVisitor::visit(FuncExpression* node) {
    matcher = std::make_unique<FuncNameMatcher>(node);
}

void ASTMakeMatcherVisitor::visit(PragmaClangExprNode* node) {
    matcher = std::make_unique<PragmaClangMatcher>(node);
}

void ASTMakeMatcherVisitor::visit(NotationExprNode* node) {
    matcher = std::make_unique<NotationMatcher>(node);
}

void ASTMakeMatcherVisitor::visit(NotationAnalysisExprNode* node) {
    matcher = std::make_unique<NotationAnalysisMatcher>(node);
}
