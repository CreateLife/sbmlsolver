/*
 * rrLLVMModelDataIRBuilder.cpp
 *
 *  Created on: Jun 28, 2013
 *      Author: andy
 */
#include "rrLLVMModelDataIRBuilder.h"
#include "lsLibStructural.h"
#include "rrLLVMException.h"
#include "rrOSSpecifics.h"

#include <sbml/Model.h>
#include <sbml/SBMLDocument.h>

#include <string>
#include <vector>
#include <sstream>

#include "llvm/ADT/APInt.h"


using namespace libsbml;
using namespace llvm;
using namespace std;

namespace rr
{

const char* LLVMModelDataIRBuilder::ModelDataName = "rr::ModelData";
const char* LLVMModelDataIRBuilder::csr_matrixName = "rr::csr_matrix";
const char* LLVMModelDataIRBuilder::csr_matrix_set_nzName = "rr::csr_matrix_set_nz";
const char* LLVMModelDataIRBuilder::csr_matrix_get_nzName = "rr::csr_matrix_get_nz";

LLVMModelDataIRBuilder::LLVMModelDataIRBuilder(Value *modelData,
        const LLVMModelDataSymbols &symbols,
        IRBuilder<> &b) :
        modelData(modelData),
        builder(b),
        symbols(symbols)

{
    validateStruct(modelData, __FUNC__);
}




llvm::Value* LLVMModelDataIRBuilder::createGlobalParamGEP(const std::string& id)
{
    uint index = symbols.getGlobalParameterIndex(id);
    assert(index < symbols.getIndependentGlobalParameterSize());
    return createGEP(GlobalParameters, index);
}


llvm::Value* LLVMModelDataIRBuilder::createGEP(ModelDataFields field,
        const Twine& name)
{
    const char* fieldName = LLVMModelDataSymbols::getFieldName(field);
    return builder.CreateStructGEP(modelData, (unsigned)field, Twine(fieldName) + Twine("_gep"));
}

llvm::Value* LLVMModelDataIRBuilder::createFloatSpeciesCompGEP(const std::string& id)
{
    int compIndex = symbols.getFloatingSpeciesCompartmentIndex(id);
    return createGEP(CompartmentVolumes, compIndex);
}

llvm::Value* LLVMModelDataIRBuilder::createGEP(ModelDataFields field,
        unsigned index, const Twine& name)
{
    Twine fieldName = LLVMModelDataSymbols::getFieldName(field);
    Value *fieldGEP = createGEP(field, fieldName + "_gep");
    Value *load = builder.CreateLoad(fieldGEP, fieldName + "_load");
    return builder.CreateConstGEP1_32(load, index, name + "_gep");
}




llvm::StructType* LLVMModelDataIRBuilder::getCSRSparseStructType(
        llvm::Module* module, llvm::ExecutionEngine* engine)
{
    StructType *structType = module->getTypeByName(csr_matrixName);

    if (!structType)
    {
        LLVMContext &context = module->getContext();

        vector<Type*> elements;

        elements.push_back(Type::getInt32Ty(context));                        // int m;
        elements.push_back(Type::getInt32Ty(context));                        // int n;
        elements.push_back(Type::getInt32Ty(context));                        // int nnz;
        elements.push_back(Type::getDoublePtrTy(context));                    // double* values;
        elements.push_back(Type::getInt32PtrTy(context));                     // int* colidx;
        elements.push_back(Type::getInt32PtrTy(context));                     // int* rowptr;

        structType = StructType::create(context, elements, csr_matrixName);

        if (engine)
        {
            const DataLayout *dl = engine->getDataLayout();

            size_t llvm_size = dl->getTypeStoreSize(structType);

            if (sizeof(csr_matrix) != llvm_size)
            {
                stringstream err;
                err << "llvm " << csr_matrixName << " size " << llvm_size <<
                        " does NOT match C++ sizeof(dcsr_matrix) " <<
                        sizeof(ModelData);
                throw LLVMException(err.str(), __FUNC__);
            }
        }
    }
    return structType;
}

llvm::Function* LLVMModelDataIRBuilder::getCSRMatrixSetNZDecl(Module *module)
{
    Function *f = module->getFunction(
            LLVMModelDataIRBuilder::csr_matrix_set_nzName);

    if (f == 0)
    {
        // bool csr_matrix_set_nz(csr_matrix *mat, int row, int col, double val);
        Type *args[] = {
                getCSRSparseStructType(module)->getPointerTo(),
                Type::getInt32Ty(module->getContext()),
                Type::getInt32Ty(module->getContext()),
                Type::getDoubleTy(module->getContext())
        };
        FunctionType *funcType = FunctionType::get(
                IntegerType::get(module->getContext(), sizeof(bool) * 8), args,
                false);
        f = Function::Create(funcType, Function::InternalLinkage,
                LLVMModelDataIRBuilder::csr_matrix_set_nzName, module);
    }
    return f;
}

llvm::Function* LLVMModelDataIRBuilder::getCSRMatrixGetNZDecl(Module *module)
{
    Function *f = module->getFunction(
            LLVMModelDataIRBuilder::csr_matrix_get_nzName);

    if (f == 0)
    {
        // double csr_matrix_get_nz(const csr_matrix* mat, int row, int col)
        Type *args[] = {
                getCSRSparseStructType(module)->getPointerTo(),
                Type::getInt32Ty(module->getContext()),
                Type::getInt32Ty(module->getContext())
        };
        FunctionType *funcType = FunctionType::get(
                Type::getDoubleTy(module->getContext()), args, false);
        f = Function::Create(funcType, Function::InternalLinkage,
                LLVMModelDataIRBuilder::csr_matrix_get_nzName, module);
    }
    return f;
}

llvm::CallInst* LLVMModelDataIRBuilder::createCSRMatrixSetNZ(IRBuilder<> &builder,
        llvm::Value* csrPtr, llvm::Value* row, llvm::Value* col,
        llvm::Value* value, const Twine& name)
{
    Function *func = LLVMModelDataIRBuilder::getCSRMatrixSetNZDecl(getModule(builder, __FUNC__));
    Value *args[] = {csrPtr, row, col, value};
    return builder.CreateCall(func, args, name);
}

llvm::Value* LLVMModelDataIRBuilder::createFloatSpeciesAmtGEP(
        const std::string& id, const Twine& name)
{
    uint index = symbols.getFloatingSpeciesIndex(id);
    assert(index < symbols.getIndependentFloatingSpeciesSize());
    return createGEP(FloatingSpeciesAmounts, index,
            name.isTriviallyEmpty() ? id : name);
}

llvm::Value* LLVMModelDataIRBuilder::createFloatSpeciesAmtLoad(
        const std::string& id, const llvm::Twine& name)
{
    Value *gep = createFloatSpeciesAmtGEP(id, name + "_gep");
    return builder.CreateLoad(gep, name);
}

llvm::Value* LLVMModelDataIRBuilder::createFloatSpeciesAmtStore(
        const std::string& id, llvm::Value* value)
{
    Value *gep = createFloatSpeciesAmtGEP(id);
    return builder.CreateStore(value, gep);
}

llvm::Module* LLVMModelDataIRBuilder::getModule(IRBuilder<> &builder, const char* func)
{
    BasicBlock *bb = 0;
    if((bb = builder.GetInsertBlock()) != 0)
    {
        Function *function = bb->getParent();
        if(function)
        {
            return function->getParent();
        }
    }
    throw LLVMException("could not get module, a BasicBlock is not currently being populated.", func);
}


llvm::CallInst* LLVMModelDataIRBuilder::createCSRMatrixGetNZ(IRBuilder<> &builder,
        llvm::Value* csrPtr, llvm::Value* row, llvm::Value* col,
        const Twine& name)
{
    Function *func = LLVMModelDataIRBuilder::getCSRMatrixGetNZDecl(getModule(builder, __FUNC__));
    Value *args[] = {csrPtr, row, col};
    return builder.CreateCall(func, args, name);
}


llvm::Value* LLVMModelDataIRBuilder::createLoad(ModelDataFields field, unsigned index, const llvm::Twine& name)
{
    Value *gep = this->createGEP(field, index, name);
    return builder.CreateLoad(gep, name);
}

llvm::Value* LLVMModelDataIRBuilder::createRateRuleGEP(const std::string& id,
        const llvm::Twine& name)
{
    uint index = symbols.getRateRuleIndex(id);
    assert(index < symbols.getRateRuleSize());
    return createGEP(RateRules, index,
            name.isTriviallyEmpty() ? id : name);
}

llvm::Value* LLVMModelDataIRBuilder::createRateRuleLoad(const std::string& id,
        const llvm::Twine& name)
{
    Value *gep = createRateRuleGEP(id);
    Twine loadName = (name.isTriviallyEmpty() ? id : name) + "_load";
    return builder.CreateLoad(gep, loadName);
}

llvm::Value* LLVMModelDataIRBuilder::createRateRuleStore(const std::string& id,
        llvm::Value* value)
{
    Value *gep = createRateRuleGEP(id);
    return builder.CreateStore(value, gep);
}

llvm::Value* LLVMModelDataIRBuilder::createStore(ModelDataFields field,
        unsigned index, llvm::Value* value, const Twine& name)
{
    Value *gep = this->createGEP(field, index, name);
    return builder.CreateStore(value, gep);
}

llvm::Value* LLVMModelDataIRBuilder::createCompLoad(const std::string& id,
        const llvm::Twine& name)
{
    Value *gep = createCompGEP(id, name + "_gep");
    return builder.CreateLoad(gep, name);
}

llvm::Value* LLVMModelDataIRBuilder::createCompStore(const std::string& id,
        llvm::Value* value)
{
    Value *gep = createCompGEP(id);
    return builder.CreateStore(value, gep);
}

llvm::Value* LLVMModelDataIRBuilder::createCompGEP(const std::string& id,
        const llvm::Twine& name)
{
    uint index = symbols.getCompartmentIndex(id);
    assert(index < symbols.getIndependentCompartmentSize());
    return createGEP(CompartmentVolumes, index,
            name.isTriviallyEmpty() ? id : name);
}



llvm::Value* LLVMModelDataIRBuilder::createBoundSpeciesAmtLoad(const std::string& id, const llvm::Twine& name)
{
    Value *gep = createBoundSpeciesAmtGEP(id, name + "_gep");
    return builder.CreateLoad(gep, name);
}

llvm::Value* LLVMModelDataIRBuilder::createBoundSpeciesAmtStore(
        const std::string& id, llvm::Value* value)
{
    Value *gep = createBoundSpeciesAmtGEP(id);
    return builder.CreateStore(value, gep);
}

llvm::Value* LLVMModelDataIRBuilder::createBoundSpeciesAmtGEP(
        const std::string& id, const llvm::Twine& name)
{
    uint index = symbols.getBoundarySpeciesIndex(id);
    assert(index < symbols.getIndependentBoundarySpeciesSize());
    return createGEP(BoundarySpeciesAmounts, index, name);
}

llvm::Value* LLVMModelDataIRBuilder::createBoundSpeciesCompGEP(const std::string& id)
{
    int compIndex = symbols.getBoundarySpeciesCompartmentIndex(id);
    return createGEP(CompartmentVolumes, compIndex);
}

llvm::Value* LLVMModelDataIRBuilder::createGlobalParamLoad(
        const std::string& id, const llvm::Twine& name)
{
    int idx = symbols.getGlobalParameterIndex(id);
    return createLoad(GlobalParameters, idx,
            name.isTriviallyEmpty() ? id : name);
}

llvm::Value* LLVMModelDataIRBuilder::createGlobalParamStore(
        const std::string& id, llvm::Value* value)
{
    int idx = symbols.getGlobalParameterIndex(id);
    return createStore(GlobalParameters, idx, value, id);
}

llvm::Value* LLVMModelDataIRBuilder::createReactionRateLoad(const std::string& id, const llvm::Twine& name)
{
    int idx = symbols.getReactionIndex(id);
    return createLoad(ReactionRates, idx, name);
}

llvm::Value* LLVMModelDataIRBuilder::createReactionRateStore(const std::string& id, llvm::Value* value)
{
    int idx = symbols.getReactionIndex(id);
    return createStore(ReactionRates, idx, value, id);
}

void LLVMModelDataIRBuilder::validateStruct(llvm::Value* s,
        const char* funcName)
{
    PointerType *pointerType = dyn_cast<PointerType>(s->getType());

    // get the type the pointer refrerences
    Type *type = pointerType ? pointerType->getElementType() : s->getType();

    StructType *structType = dyn_cast<StructType>(type);

    if (structType)
    {
        if (structType->getName().compare(ModelDataName) == 0)
        {
            return;
        }
    }
    std::string str;
    raw_string_ostream err(str);
    err << "error in " << funcName << ", " << "Invalid argument type, expected "
            << ModelDataName << ", but received ";
    type->print(err);
    throw LLVMException(err.str());
}

llvm::StructType *LLVMModelDataIRBuilder::getStructType(llvm::Module *module, llvm::ExecutionEngine *engine)
{
    StructType *structType = module->getTypeByName("rr::ModelData");

    if (!structType)
    {
        LLVMContext &context = module->getContext();

        // different compilers define bool differently, it used to be more common to store
        // bool as a 32 bit int, but now its more common to use an 8 bit int.
        Type *boolType = IntegerType::get(context, sizeof(bool) * 8);
        Type *boolPtrType = boolType->getPointerTo();
        Type *charStarStarType = Type::getInt8PtrTy(context)->getPointerTo();
        Type *voidPtrType = Type::getInt8PtrTy(context);
        Type *csrSparseType = getCSRSparseStructType(module, engine);
        Type *csrSparsePtrType = csrSparseType->getPointerTo();

        vector<Type*> elements;

        elements.push_back(Type::getInt32Ty(context));        // 0      unsigned                            size;
        elements.push_back(Type::getInt32Ty(context));        // 1      unsigned                            flags;
        elements.push_back(Type::getDoubleTy(context));       // 2      double                              time;
        elements.push_back(Type::getInt32Ty(context));        // 3      int                                 numIndependentSpecies;
        elements.push_back(Type::getInt32Ty(context));        // 4      int                                 numDependentSpecies;
        elements.push_back(Type::getDoublePtrTy(context));    // 5      double*                             dependentSpeciesConservedSums;
        elements.push_back(Type::getInt32Ty(context));        // 6      int                                 numGlobalParameters;
        elements.push_back(Type::getDoublePtrTy(context));    // 7      double*                             globalParameters;
        elements.push_back(Type::getInt32Ty(context));        // 8      int                                 numReactions;
        elements.push_back(Type::getDoublePtrTy(context));    // 9      ouble*                              reactionRates;
        elements.push_back(Type::getInt32Ty(context));        // 10     int                                 numRateRules;
        elements.push_back(Type::getDoublePtrTy(context));    // 11     double*                             rateRules;
        elements.push_back(Type::getInt32PtrTy(context));     // 12     int*                                localParametersOffsets;
        elements.push_back(Type::getInt32PtrTy(context));     // 13     int*                                localParametersNum;
        elements.push_back(Type::getDoublePtrTy(context));    // 14     double*                             localParameters;
        elements.push_back(Type::getInt32Ty(context));        // 15     int                                 numFloatingSpecies;
        elements.push_back(Type::getDoublePtrTy(context));    // 16     double*                             floatingSpeciesConcentrations;
        elements.push_back(Type::getDoublePtrTy(context));    // 17     double*                             floatingSpeciesInitConcentrations;
        elements.push_back(Type::getDoublePtrTy(context));    // 18     double*                             floatingSpeciesAmountRates;
        elements.push_back(Type::getDoublePtrTy(context));    // 19     double*                             floatingSpeciesAmounts;
        elements.push_back(Type::getInt32PtrTy(context));     // 20     int*                                floatingSpeciesCompartments;
        elements.push_back(Type::getInt32Ty(context));        // 21     int                                 numBoundarySpecies;
        elements.push_back(Type::getDoublePtrTy(context));    // 22     double*                             boundarySpeciesConcentrations;
        elements.push_back(Type::getDoublePtrTy(context));    // 23     double*                             boundarySpeciesAmounts;
        elements.push_back(Type::getInt32PtrTy(context));     // 24     int*                                boundarySpeciesCompartments;
        elements.push_back(Type::getInt32Ty(context));        // 25     int                                 numCompartments;
        elements.push_back(Type::getDoublePtrTy(context));    // 26     double*                             compartmentVolumes;
        elements.push_back(csrSparsePtrType);                 // 27     dcsr_matrix                         stoichiometry;
        elements.push_back(Type::getInt32Ty(context));        // 28     int                                 numEvents;
        elements.push_back(Type::getInt32Ty(context));        // 39     int                                 eventTypeSize;
        elements.push_back(boolPtrType);                      // 30     bool*                               eventType;
        elements.push_back(Type::getInt32Ty(context));        // 31     int                                 eventPersistentTypeSize;
        elements.push_back(boolPtrType);                      // 32     bool*                               eventPersistentType;
        elements.push_back(Type::getInt32Ty(context));        // 33     int                                 eventTestsSize;
        elements.push_back(Type::getDoublePtrTy(context));    // 34     double*                             eventTests;
        elements.push_back(Type::getInt32Ty(context));        // 35     int                                 eventPrioritiesSize;
        elements.push_back(Type::getDoublePtrTy(context));    // 36     double*                             eventPriorities;
        elements.push_back(Type::getInt32Ty(context));        // 37     int                                 eventStatusArraySize;
        elements.push_back(boolPtrType);                      // 38     bool*                               eventStatusArray;
        elements.push_back(Type::getInt32Ty(context));        // 39     int                                 previousEventStatusArraySize;
        elements.push_back(boolPtrType);                      // 40     bool*                               previousEventStatusArray;
        elements.push_back(Type::getInt32Ty(context));        // 41     int                                 stateVectorSize;
        elements.push_back(Type::getDoublePtrTy(context));    // 42     double*                             stateVector;
        elements.push_back(Type::getDoublePtrTy(context));    // 43     double*                             stateVectorRate;
        elements.push_back(Type::getInt32Ty(context));        // 44     int                                 workSize;
        elements.push_back(Type::getDoublePtrTy(context));    // 45     double*                             work;
        elements.push_back(voidPtrType);                      // 46     EventDelayHandler*                eventDelays;
        elements.push_back(voidPtrType);                      // 47     EventAssignmentHandler*           eventAssignments;
        elements.push_back(voidPtrType);                      // 48     ComputeEventAssignmentHandler*    computeEventAssignments;
        elements.push_back(voidPtrType);                      // 49     PerformEventAssignmentHandler*    performEventAssignments;
        elements.push_back(Type::getInt8PtrTy(context));      // 50     char*                               modelName;
        elements.push_back(Type::getInt32Ty(context));        // 51     int                                 srSize;
        elements.push_back(Type::getDoublePtrTy(context));    // 52     double*                             sr;

        structType = StructType::create(context, elements, "rr::ModelData");

        if (engine)
        {
            const DataLayout *dl = engine->getDataLayout();

            size_t llvm_size = dl->getTypeStoreSize(structType);

            if (sizeof(ModelData) != llvm_size)
            {
                stringstream err;
                err << "llvm rr::ModelData size " << llvm_size <<
                        " does NOT match C++ sizeof(ModelData) " <<
                        sizeof(ModelData);
                throw LLVMException(err.str(), __FUNC__);
            }

            /*
            printf("TestStruct size: %i, , LLVM Size: %i\n", sizeof(ModelData), llvm_size);

            printf("C++ bool size: %i, LLVM bool size: %i\n", sizeof(bool), dl->getTypeStoreSize(boolType));
            printf("C++ bool* size: %i, LLVM bool* size: %i\n", sizeof(bool*), dl->getTypeStoreSize(boolPtrType));
            printf("C++ char** size: %i, LLVM char** size: %i\n", sizeof(char**), dl->getTypeStoreSize(charStarStarType));
            printf("C++ void* size: %i, LLVM void* size: %i\n", sizeof(void*), dl->getTypeStoreSize(voidPtrType));

            printf("C++ EventDelayHandler* size: %i\n", sizeof(EventDelayHandler*));
            printf("C++ EventAssignmentHandler* size: %i\n", sizeof(EventAssignmentHandler* ));
            printf("C++ ComputeEventAssignmentHandler* size: %i\n", sizeof(ComputeEventAssignmentHandler*));
            printf("C++ PerformEventAssignmentHandler* size: %i\n", sizeof(PerformEventAssignmentHandler*));

            */
        }
    }
    return structType;
}








/*****************************************************************************/
/*****************************************************************************/
/*****************              TESTING STUFF              *******************/
/*****************************************************************************/
/*****************************************************************************/



LLVMModelDataIRBuilderTesting::LLVMModelDataIRBuilderTesting(LLVMModelDataSymbols const& symbols,
        llvm::IRBuilder<> &b) :
        symbols(symbols),
        builder(b)
{
}


void LLVMModelDataIRBuilderTesting::createAccessors(Module *module)
{
    const string getSizeName = "get_size";
    Function* getSizeFunc = module->getFunction(getSizeName);
    if(!getSizeFunc)
    {
        LLVMContext &context = module->getContext();
        StructType *structType = LLVMModelDataIRBuilder::getStructType(module);
        vector<Type*> getArgTypes(1, PointerType::get(structType, 0));
        FunctionType *getFuncType = FunctionType::get(Type::getInt32Ty(context),
                getArgTypes, false);
        getSizeFunc = Function::Create(getFuncType, Function::ExternalLinkage,
                getSizeName, module);

        BasicBlock *getBlock = BasicBlock::Create(context, "entry", getSizeFunc);
        builder.SetInsertPoint(getBlock);
        vector<Value*> getArgValues;
        for(Function::arg_iterator i = getSizeFunc->arg_begin();
                i != getSizeFunc->arg_end(); i++)
        {
            Value *v = i;
            //v->dump();
            getArgValues.push_back(i);
        }

        LLVMModelDataIRBuilder mdbuilder (getArgValues[0], symbols, builder);



        Value *gep = mdbuilder.createGEP(Size);

        Value *getRet = builder.CreateLoad(gep);

        builder.CreateRet(getRet);

        verifyFunction(*getSizeFunc);

        //getSizeFunc->dump();

    }

}

llvm::CallInst* LLVMModelDataIRBuilderTesting::createDispChar(llvm::Value* intVal)
{
    return builder.CreateCall(getDispCharDecl(LLVMModelDataIRBuilder::getModule(builder, __FUNC__)), intVal);
}

llvm::CallInst* LLVMModelDataIRBuilderTesting::createDispDouble(llvm::Value* doubleVal)
{
    return builder.CreateCall(getDispDoubleDecl(LLVMModelDataIRBuilder::getModule(builder, __FUNC__)), doubleVal);
}

llvm::CallInst* LLVMModelDataIRBuilderTesting::createDispInt(llvm::Value* intVal)
{
    return builder.CreateCall(getDispIntDecl(LLVMModelDataIRBuilder::getModule(builder, __FUNC__)), intVal);
}


pair<Function*, Function*> LLVMModelDataIRBuilderTesting::createFloatingSpeciesAccessors(
        llvm::Module* module, const std::string id)
{
    const string getName = "get_floatingspecies_conc_" + id;
    const string setName = "set_floatingspecies_conc_" + id;

    pair<Function*, Function*> result(module->getFunction(getName),
            module->getFunction(setName));

    if (!result.first || !result.second)
    {
        LLVMContext &context = module->getContext();
        StructType *structType = LLVMModelDataIRBuilder::getStructType(module);
        vector<Type*> getArgTypes(1, PointerType::get(structType, 0));
        FunctionType *getFuncType = FunctionType::get(
                Type::getDoubleTy(context), getArgTypes, false);
        result.first = Function::Create(getFuncType, Function::ExternalLinkage,
                getName, module);

        BasicBlock *block = BasicBlock::Create(context, "entry",
                result.first);
        builder.SetInsertPoint(block);
        vector<Value*> getArgValues;
        for (Function::arg_iterator i = result.first->arg_begin();
                i != result.first->arg_end(); i++)
        {
            Value *v = i;
            //v->dump();
            getArgValues.push_back(i);

        }

        LLVMModelDataIRBuilder mdbuilder (getArgValues[0], symbols, builder);


        //Value *getEP = mdbuilder.createFloatSpeciesConcGEP(id);

        //Value *getRet = builder.CreateLoad(getEP);

        //builder.CreateRet(getRet);

        verifyFunction(*result.first);

        //result.first->dump();

        vector<Type*> setArgTypes;
        setArgTypes.push_back(PointerType::get(structType, 0));
        setArgTypes.push_back(Type::getDoubleTy(context));
        FunctionType *setFuncType = FunctionType::get(Type::getVoidTy(context),
                setArgTypes, false);
        result.second = Function::Create(setFuncType, Function::ExternalLinkage,
                setName, module);

        block = BasicBlock::Create(context, "entry",
                result.second);
        builder.SetInsertPoint(block);
        vector<Value*> setArgValues;
        for (Function::arg_iterator i = result.second->arg_begin();
                i != result.second->arg_end(); i++)
        {
            Value *v = i;
            //v->dump();
            setArgValues.push_back(i);

        }


        //Value *val = mdbuilder.createFloatSpeciesConcStore(id, setArgValues[1]);

        builder.CreateRetVoid();

        verifyFunction(*result.second);

        //result.second->dump();

        cout << "pause...\n";

    }

    return result;
}

llvm::Function* LLVMModelDataIRBuilderTesting::getDispCharDecl(llvm::Module* module)
{
    Function *f = module->getFunction("dispChar");
    if (f == 0) {
        std::vector<Type*> args(1, Type::getInt8Ty(module->getContext()));
        FunctionType *funcType = FunctionType::get(Type::getVoidTy(module->getContext()), args, false);
        f = Function::Create(funcType, Function::InternalLinkage, "dispChar", module);
    }
    return f;
}



llvm::Function* LLVMModelDataIRBuilderTesting::getDispDoubleDecl(llvm::Module* module)
{
    Function *f = module->getFunction("dispDouble");
    if (f == 0) {
        std::vector<Type*> args(1, Type::getDoubleTy(module->getContext()));
        FunctionType *funcType = FunctionType::get(Type::getVoidTy(module->getContext()), args, false);
        f = Function::Create(funcType, Function::InternalLinkage, "dispDouble", module);
    }
    return f;
}


llvm::Function* LLVMModelDataIRBuilderTesting::getDispIntDecl(llvm::Module* module)
{
    Function *f = module->getFunction("dispInt");
    if (f == 0) {
        std::vector<Type*> args(1, Type::getInt32Ty(module->getContext()));
        FunctionType *funcType = FunctionType::get(Type::getVoidTy(module->getContext()), args, false);
        f = Function::Create(funcType, Function::InternalLinkage, "dispInt", module);
    }
    return f;
}

llvm::Value* LLVMModelDataIRBuilderTesting::createFloatSpeciesConcGEP(const std::string& id)
{
    //int index = symbols.getFloatingSpeciesIndex(id);
    //return createGEP(FloatingSpeciesConcentrations, index);
    return 0;
}

llvm::Value* LLVMModelDataIRBuilderTesting::createFloatSpeciesConcStore(const string& id, Value *conc)
{
    //int index = symbols.getFloatingSpeciesIndex(id);

    //Value *compEP = createFloatSpeciesCompGEP(id);
    //Value *volume = builder.CreateLoad(compEP);
    //Value *amount = builder.CreateFMul(conc, volume);

    //builder.CreateStore(amount, createGEP(FloatingSpeciesAmounts, index));
    //return builder.CreateStore(conc, createGEP(FloatingSpeciesConcentrations, index));
    return 0;

}



void LLVMModelDataIRBuilderTesting::test(Module *module, IRBuilder<> *build,
        ExecutionEngine * engine)
{
//    TheModule = module;
//    context = &module->getContext();
//    rr::builder = build;
//    TheExecutionEngine = engine;
//    createDispPrototypes();
//
//    callDispInt(23);
//
//    structSetProto();
//
//    TestStruct s =
//    { 0 };
//
//    s.var3 = (char*) calloc(50, sizeof(char));
//    s.var5 = (char*) calloc(50, sizeof(char));
//
//    sprintf(s.var5, "1234567890");
//
//    char* p = (char*) calloc(50, sizeof(char));
//
//    dispStruct(&s);
//
//    callStructSet(&s, 314, 3.14, 2.78, p, 0, 0);
//
//    printf("p: %s\n", p);
//
//    dispStruct(&s);
//
//    printf("new var5: ");
//    for (int i = 0; i < 10; i++)
//    {
//        printf("{i:%i,c:%i}, ", i, s.var5[i]);
//    }
//    printf("\n");
}




//
//
//static Module *TheModule;
//static IRBuilder<> *builder;
//static ExecutionEngine *TheExecutionEngine;
//static LLVMContext *context;
//
//static LLVMContext &getContext() {
//    return *context;
//}
//
//Value *storeInStruct(IRBuilder<> *B, Value *S, Value *V, unsigned index, const char* s = "");
//Value *loadFromStruct(IRBuilder<> *B, Value *S, unsigned index, const char* s = "");
//
//struct TestStruct {
//    int var0;
//    double var1;
//    double var2;
//    char* var3;
//    double var4;
//    char* var5;
//};
//
//StructType* getTestStructType() {
//    // static StructType *     create (ArrayRef< Type * > Elements, StringRef Name, bool isPacked=false)
//    vector<Type*> elements;
//    elements.push_back(Type::getInt32Ty(getContext()));   // int var0;
//    elements.push_back(Type::getDoubleTy(getContext()));  // double var1;
//    elements.push_back(Type::getDoubleTy(getContext()));  // double var2;
//    //elements.push_back(ArrayType::get(Type::getInt8Ty(getContext()), 0)); // char* var3;
//    elements.push_back(Type::getInt8PtrTy(getContext())); // char* var5;
//    elements.push_back(Type::getDoubleTy(getContext()));  // double var4;
//    //elements.push_back(ArrayType::get(Type::getInt8Ty(getContext()), 0));   // char[] var5;
//    elements.push_back(Type::getInt8PtrTy(getContext())); // char* var5; // char* var5;
//
//
//
//
//    StructType *s = StructType::create(elements, "TestStruct");
//
//    const DataLayout &dl = *TheExecutionEngine->getDataLayout();
//
//    size_t llvm_size = dl.getTypeStoreSize(s);
//
//    printf("TestStruct size: %i, , LLVM Size: %i\n", sizeof(TestStruct), llvm_size);
//
//
//
//    return s;
//}
//
//Value *getVar5(Value *testStructPtr, int index) {
//    GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(builder->CreateStructGEP(testStructPtr, 5, "var5_gep"));
//    LoadInst * var5_load = builder.CreateLoad(gep, "var5_load");
//    //Value *var5_load = loadFromStruct(Builder, testStructPtr, 5, "var5");
//    gep = dyn_cast<GetElementPtrInst>(builder->CreateConstGEP1_32(var5_load, index, "var5_elem_gep"));
//    return builder.CreateLoad(gep, "var5_elem");
//}
//
//
//static void dispStruct(TestStruct *s)
//{
//    printf("%s {\n",  __PRETTY_FUNCTION__);
//    printf("\tvar0: %i\n", s->var0);
//    printf("\tvar1: %f\n", s->var1);
//    printf("\tvar2: %f\n", s->var2);
//    printf("\tvar3: %s\n", s->var3);
//    printf("\tvar4: %f\n", s->var4);
//    printf("\tvar5: %s\n", s->var5);
//    printf("}\n");
//
//
//}
//
//static Function* dispStructProto() {
//
//    StructType *structType = getTestStructType();
//    PointerType *structTypePtr = llvm::PointerType::get(structType, 0);
//    std::vector<Type*> args(1, structTypePtr);
//    FunctionType *funcType = FunctionType::get(Type::getVoidTy(getContext()), args, false);
//    Function *f = Function::Create(funcType, Function::InternalLinkage, "dispStruct", TheModule);
//    return f;
//}
//
//static void dispInt(int i) {
//    printf("%s, %i\n", __PRETTY_FUNCTION__, i);
//}
//

//
//static void callDispInt(int val) {
//    // Evaluate a top-level expression into an anonymous function.
//
//    Function *func = TheExecutionEngine->FindFunctionNamed("dispInt");
//
//    // JIT the function, returning a function pointer.
//    void *fptr = TheExecutionEngine->getPointerToFunction(func);
//
//    // Cast it to the right type (takes no arguments, returns a double) so we
//    // can call it as a native function.
//    void (*fp)(int) = (void (*)(int))fptr;
//
//    fp(val);
//}
//
//
//

//
//static void dispCharStar(char* p) {
//    printf("%s: %s\n", __PRETTY_FUNCTION__, p);
//}
//
//static Function* dispCharStarProto() {
//    Function *f = TheModule->getFunction("dispCharStar");
//    if (f == 0) {
//        std::vector<Type*>args(1, Type::getInt8PtrTy(getContext()));
//        FunctionType *funcType = FunctionType::get(Type::getVoidTy(getContext()), args, false);
//        f = Function::Create(funcType, Function::InternalLinkage, "dispCharStar", TheModule);
//    }
//    return f;
//}
//
//static void dispChar(char p) {
//    printf("%s as char: %c\n", __PRETTY_FUNCTION__, p);
//    printf("%s as int: %i\n", __PRETTY_FUNCTION__, (int)p);
//}
//
//static Function* dispCharProto() {
//    Function *f = TheModule->getFunction("dispChar");
//    if (f == 0) {
//        std::vector<Type*>args(1, Type::getInt8Ty(getContext()));
//        FunctionType *funcType = FunctionType::get(Type::getVoidTy(getContext()), args, false);
//        f = Function::Create(funcType, Function::InternalLinkage, "dispChar", TheModule);
//    }
//    return f;
//}
//
//static void createDispPrototypes() {
//    TheExecutionEngine->addGlobalMapping(dispStructProto(), (void*)dispStruct);
//    TheExecutionEngine->addGlobalMapping(dispIntProto(), (void*)dispInt);
//    TheExecutionEngine->addGlobalMapping(dispDoubleProto(), (void*)dispDouble);
//    TheExecutionEngine->addGlobalMapping(dispCharStarProto(), (void*)dispCharStar);
//    TheExecutionEngine->addGlobalMapping(dispCharProto(), (void*)dispChar);
//}
//
//static void structSetBody(Function *func) {
//    // Create a new basic block to start insertion into.
//    BasicBlock *BB = BasicBlock::Create(getContext(), "entry", func);
//    builder.SetInsertPoint(BB);
//
//    LLVMContext &context = getContext();
//
//    std::vector<Value*> args;
//
//    // Set names for all arguments.
//    unsigned idx = 0;
//    for (Function::arg_iterator ai = func->arg_begin(); ai != func->arg_end();
//            ++ai, ++idx) {
//
//        args.push_back(ai);
//        //ai->setName(Args[Idx]);
//
//        // Add arguments to variable symbol table.
//        //NamedValues[Args[Idx]] = AI;
//    }
//
//    Value *arg = ConstantInt::get(Type::getInt32Ty(getContext()), 16);
//
//    builder.CreateCall(dispIntProto(), args[1], "");
//
//    builder.CreateCall(dispDoubleProto(), args[2], "");
//
//    //builder->CreateCall(dispDoubleProto(), args[3], "");
//
//    builder.CreateCall(dispCharStarProto(), args[4], "");
//
//    storeInStruct(builder, args[0], args[1], 0);
//    storeInStruct(builder, args[0], args[2], 1);
//    storeInStruct(builder, args[0], args[3], 2);
//    storeInStruct(builder, args[0], args[5], 4);
//
//
//
//    Value *var3 = loadFromStruct(builder, args[0], 3, "var3");
//
//    //Value *var3 = args[4];
//
//    Type *t = var3->getType();
//
//    var3->dump();
//    t->dump();
//
//
//    Value *c = ConstantInt::get(Type::getInt8Ty(getContext()), 's');
//    Value *gep = builder.CreateConstGEP1_32(var3, 0, "array gep");
//    builder.CreateStore(c, gep);
//    gep = builder.CreateConstGEP1_32(var3, 1, "array gep");
//    builder.CreateStore(c, gep);
//    gep = builder.CreateConstGEP1_32(var3, 2, "array gep");
//    builder.CreateStore(c, gep);
//
//
//    c = ConstantInt::get(Type::getInt8Ty(getContext()), '2');
//
//    Value *var5_2 = getVar5(args[0], 2);
//
//    var5_2->dump();
//
//    builder.CreateCall(dispCharProto(), var5_2);
//
//
//
//    builder.CreateCall(dispCharStarProto(), args[4], "");
//
//
//
//
//    // Finish off the function.
//    builder.CreateRetVoid();
//
//    // Validate the generated code, checking for consistency.
//    verifyFunction(*func);
//}
//
//static Function* structSetProto() {
//    Function *f = TheModule->getFunction("structSet");
//
//    if (f == 0) {
//        vector<Type*> args;
//        StructType *structType = getTestStructType();
//        PointerType *structTypePtr = llvm::PointerType::get(structType, 0);
//        args.push_back(structTypePtr);
//        args.push_back(Type::getInt32Ty(getContext()));   // int var0;
//        args.push_back(Type::getDoubleTy(getContext()));  // double var1;
//        args.push_back(Type::getDoubleTy(getContext()));  // double var2;
//        args.push_back(Type::getInt8PtrTy(getContext())); // char* var3;
//        args.push_back(Type::getDoubleTy(getContext()));  // double var4;
//        args.push_back(Type::getInt8PtrTy(getContext())); // char* var5;
//
//        FunctionType *funcType = FunctionType::get(Type::getVoidTy(getContext()), args, false);
//        f = Function::Create(funcType, Function::InternalLinkage, "structSet", TheModule);
//
//
//        structSetBody(f);
//
//    }
//    return f;
//}
//
//static void callStructSet(TestStruct *s, int var0, double var1, double var2,
//        char* var3, double var4, char* var5)
//{
//    Function *func = TheExecutionEngine->FindFunctionNamed("structSet");
//
//    // JIT the function, returning a function pointer.
//    void *fptr = TheExecutionEngine->getPointerToFunction(func);
//
//    // Cast it to the right type (takes no arguments, returns a double) so we
//    // can call it as a native function.
//    void (*fp)(TestStruct*, int, double, double, char*, double, char*) =
//            (void (*)(TestStruct*, int, double, double,char*, double, char*))fptr;
//
//    fp(s, var0, var1, var2, var3, var4, var5);
//
//
//
//}
//
//
//
//// Store V in structure S element index
//Value *storeInStruct(IRBuilder<> *B, Value *S, Value *V, unsigned index, const char* s)
//{
//    return B->CreateStore(V, B->CreateStructGEP(S, index, s), s);
//}
//
//// Store V in structure S element index
//Value *loadFromStruct(IRBuilder<> *B, Value *S, unsigned index, const char* s)
//{
//    Value *gep = B->CreateStructGEP(S, index, s);
//    return B->CreateLoad(gep, s);
//}
//
//void dispIntrinsics(Intrinsic::ID id) {
//    Intrinsic::IITDescriptor ids[8];
//    SmallVector<Intrinsic::IITDescriptor, 8> table;
//    Intrinsic::getIntrinsicInfoTableEntries(id, table);
//
//    printf("table: %i\n", table.size());
//
//    for(unsigned i = 0; i < table.size(); i++) {
//        ids[i] = table[i];
//    }
//
//    for(unsigned i = 0; i < table.size(); i++) {
//        Intrinsic::IITDescriptor::IITDescriptorKind kind = ids[i].Kind;
//        Intrinsic::IITDescriptor::ArgKind argKind = ids[i].getArgumentKind();
//        unsigned argNum = ids[i].getArgumentNumber();
//
//        printf("kind: %i, argKind: %i, argNum: %i\n", kind, argKind, argNum);
//
//    }
//
//    printf("foo\n");
//
//    std::vector<Type*> args(table.size() - 1, Type::getDoubleTy(getContext()));
//
//    printf("intrinsic name: %s\n", getName(id, args).c_str());
//
//    Function *decl = Intrinsic::getDeclaration(TheModule, id, args);
//    decl->dump();
//
//    FunctionType *func = Intrinsic::getType(getContext(), id, args);
//
//    func->dump();
//
//    printf("done\n");
//
//
//
//
//
//    //func->dump();
//}


} /* namespace rr */


