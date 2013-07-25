/*
 * TestBase.h
 *
 *  Created on: Jul 20, 2013
 *      Author: andy
 */

#ifndef TESTBASE_H_
#define TESTBASE_H_

#include "rrLLVMModelGenerator.h"
#include "rrLLVMExecutableModel.h"

#include <string>

namespace rr
{

class TestBase
{
public:
    TestBase(const std::string& version, int caseNumber);
    virtual ~TestBase();
    
    virtual bool test() {return true;}

    LLVMExecutableModel *model;
    string version;
    int caseNumber;
    string fileName;
};

} /* namespace rr */
#endif /* TESTBASE_H_ */