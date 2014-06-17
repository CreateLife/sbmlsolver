/*
 * LLVMCompiler.cpp
 *
 * Created on: Jun 3, 2013
 *
 * Author: Andy Somogyi,
 *     email decode: V1 = "."; V2 = "@"; V3 = V1;
 *     andy V1 somogyi V2 gmail V3 com
 */
#pragma hdrstop
#include "LLVMCompiler.h"
#include "rrUtils.h"
#include "LLVMIncludes.h"
#include <sstream>
#include <ctime>

using namespace rr;
namespace rrllvm
{

LLVMCompiler::LLVMCompiler()
{
}

LLVMCompiler::~LLVMCompiler()
{
}

std::string LLVMCompiler::getCompiler() const
{
    return "LLVM";
}

bool LLVMCompiler::setCompiler(const std::string& compiler)
{
    return true;
}

std::string LLVMCompiler::getCompilerLocation() const
{
    return "not used";
}

bool LLVMCompiler::setCompilerLocation(const std::string& path)
{
    return true;
}

std::string LLVMCompiler::getSupportCodeFolder() const
{
    return "not used";
}

bool LLVMCompiler::setSupportCodeFolder(const std::string& path)
{
    return true;
}

} /* namespace rr */

std::string rrllvm::LLVMCompiler::getDefaultTargetTriple()
{
    return llvm::sys::getDefaultTargetTriple();
}

std::string rrllvm::LLVMCompiler::getProcessTriple()
{
    return llvm::sys::getProcessTriple();
}

std::string rrllvm::LLVMCompiler::getHostCPUName()
{
    return llvm::sys::getHostCPUName();
}

std::string rrllvm::LLVMCompiler::getVersion()
{
    std::stringstream ss;
    ss << LLVM_VERSION_MAJOR << "." << LLVM_VERSION_MINOR;
    return ss.str();
}
