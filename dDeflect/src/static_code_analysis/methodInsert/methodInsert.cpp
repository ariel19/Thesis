//#include "clang/Driver/Options.h"
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
#include <map>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

typedef pair<string,unsigned> NamePosPair;
typedef pair<string,string> NameCodePair;
typedef std::map<string, string> NameCodeMap;

Rewriter rewriter;
NameCodeMap functionsMap;

class InserterVisitor : public RecursiveASTVisitor<InserterVisitor> {
private:
    ASTContext *astContext;

public:
    explicit InserterVisitor(CompilerInstance *CI)
      : astContext(&(CI->getASTContext()))
    {
        rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        NameCodeMap::const_iterator iterator;
        for (iterator = functionsMap.begin(); iterator != functionsMap.end(); ++iterator) {
            string currentFunctionName = func->getNameInfo().getName().getAsString();
            // TODO: additional checkups


            if(func->isThisDeclarationADefinition() && iterator->first.compare(currentFunctionName)==0){
                // std::cout<<"current function: "<<currentFunctionName<<" lookedup func: "<<iterator->first<<std::endl;
                SourceLocation currentLocation;

                Stmt * definition = func->getBody();
                for (StmtRange range = definition->children(); range; ++range){
                    Stmt * fs = *range;
                    currentLocation = fs->getLocStart();
                    break;
                }
                rewriter.InsertTextBefore(currentLocation,functionsMap[iterator->first]);
            }
        }
        return true;
    }
};

class InserterASTConsumer : public ASTConsumer {
private:
    InserterVisitor *visitor;
public:

    explicit InserterASTConsumer(CompilerInstance *CI)
        : visitor(new InserterVisitor(CI))
    { }


    virtual void HandleTranslationUnit(ASTContext &Context) {
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }
};

class InserterFrontendAction : public ASTFrontendAction {
public:
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
         std::unique_ptr<ASTConsumer> p(new InserterASTConsumer(&CI));
         return p;
    }
};

static cl::OptionCategory MyToolCategory("My tool options");

int main(int argc, const char **argv) {

    for(int i = 3; i+1< argc; i+=2){
        functionsMap[argv[i]]=argv[i+1];
    }

    CommonOptionsParser op(argc, argv, MyToolCategory);

    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    int result = Tool.run(newFrontendActionFactory<InserterFrontendAction>().get());

    std::vector<string> paths = op.getSourcePathList();
    StringRef path = (paths[0]).insert(paths[0].find('.'),"Secured");
    std::error_code EC;
    llvm::raw_fd_ostream fd(path, EC, sys::fs::F_RW);

    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(fd);

    return result;

}
