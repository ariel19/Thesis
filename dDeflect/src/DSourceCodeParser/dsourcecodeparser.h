#ifndef DSOURCECODEPARSER_H
#define DSOURCECODEPARSER_H

#include<stdint.h>
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <memory>
#include <iostream>
#include <string>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;


class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext *astContext;

public:
    explicit ExampleVisitor(CompilerInstance *CI)
      : astContext(&(CI->getASTContext()))
    {
        rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {

        if(func->isThisDeclarationADefinition()){
            Stmt * definition = func->getBody();

            StringRef sr("printf(\"HelloWorld\");\n");

            for (StmtRange range = definition->children(); range; ++range){
                Stmt * fs = *range;
                SourceLocation childStart = fs->getLocStart();
                rewriter.InsertTextBefore(childStart ,sr);
                break;
            }
        }
        return true;
    }

};


class DSourceCodeParser
{
public:
    DSourceCodeParser();
};

#endif // DSOURCECODEPARSER_H
