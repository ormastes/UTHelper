#pragma once

#include "Pointer.h"
#include "ASTNode.h"



struct ASTMakeMatcherVisitor : ASTVisitor {
    AbstractAST2MatcherPtr matcher;
    virtual void visit(PointcutDeclaration* node) ;
    virtual void visit(OrExpression* node) ;
    virtual void visit(AndExpression* node) ;
    virtual void visit(NotExpression* node) ;
    virtual void visit(ParenthesizedExpression* node) ;
    virtual void visit(FuncExpression* node) ;
    virtual void visit(PragmaClangExprNode* node) ;
    virtual void visit(NotationExprNode* node) ;
    virtual void visit(NotationAnalysisExprNode* node) ;
    AbstractAST2MatcherPtr getMatcher() ;
};