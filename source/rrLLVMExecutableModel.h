/*
 * rrLLVMExecutableModel.h
 *
 *  Created on: Jun 3, 2013
 *
 * Author: Andy Somogyi,
 *     email decode: V1 = "."; V2 = "@"; V3 = V1;
 *     andy V1 somogyi V2 gmail V3 com
 */

#ifndef rrLLVMExecutableModelH
#define rrLLVMExecutableModelH

#include "rrExecutableModel.h"
#include "rrLLVMModelDataSymbols.h"
#include "rrLLVMIncludes.h"

#include "rrLLVMEvalInitialConditionsCodeGen.h"
#include "rrLLVMEvalReactionRatesCodeGen.h"
#include "rrLLVMEvalRateRuleRatesCodeGen.h"
#include "rrLLVMGetValuesCodeGen.h"

namespace rr
{

class RR_DECLSPEC LLVMExecutableModel: public ExecutableModel
{
public:

    /**
     * the default ctor just zeros out all our private bits, then
     * the main construction is handled by the model generator.
     */
    LLVMExecutableModel();


    virtual ~LLVMExecutableModel();

    /**
     * get the name of the model
     */
    virtual string getModelName();
    virtual void setTime(double _time);
    virtual double getTime();
    virtual ModelData& getModelData();

    virtual bool getConservedSumChanged();

    virtual void setConservedSumChanged(bool);

    /**
     * evaluate the initial conditions specified in the sbml, this entails
     * evaluating all InitialAssigments, AssigmentRules, initial values, etc...
     *
     * The the model state is fully set.
     */
    virtual void evalInitialConditions();

    /**
     * reset the model to its original state
     */
    virtual void reset();

    /**
     * A ExecutableModel holds a stack of states, the entire state of this
     * model is pushed onto the saved state stack, and the current state
     * remains unchanged.
     *
     * @returns the size of the saved stack after the current state has been
     * pushed.
     */
    virtual int pushState(unsigned);

    /**
     * restore the state from a previously saved state, if the state stack
     * is empty, this has no effect.
     *
     * @returns the size of the saved stack after the top has been poped.
     */
    virtual int popState(unsigned);


    // functions --------------------------------------------------------
    virtual int getNumIndependentSpecies();
    virtual int getNumDependentSpecies();
    virtual int getNumFloatingSpecies();
    virtual int getNumBoundarySpecies();
    virtual int getNumGlobalParameters();
    virtual double getGlobalParameterValue(int index);
    virtual void setGlobalParameterValue(int index, double value);


    virtual int getNumCompartments();

    /**
     * get the global parameter values
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[out] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int getGlobalParameterValues(int len, int const *indx,
            double *values);

    virtual int getNumReactions();

    virtual int getReactionRates(int len, int const *indx,
                    double *values);

    /**
     * get the compartment volumes
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[out] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int getCompartmentVolumes(int len, int const *indx,
            double *values);

    virtual int getNumRules();
    virtual int getNumEvents();
    virtual void computeEventPriorites();

    virtual int getNumLocalParameters(int reactionId);

    virtual void evalInitialAssignments();

    virtual void convertToAmounts();
    virtual void computeConservedTotals();


    //Access dll data
    virtual void getRateRuleValues(double *rateRuleValues);



    virtual void setRateRuleValues(const double *rateRuleValues);

    /**
     * copies the internal model state vector into the provided
     * buffer.
     *
     * @param[out] stateVector a buffer to copy the state vector into, if NULL,
     *         return the size required.
     *
     * @return the number of items coppied into the provided buffer, if
     *         stateVector is NULL, returns the length of the state vector.
     */
    virtual int getStateVector(double *stateVector);

    /**
     * sets the internal model state to the provided packed state vector.
     *
     * @param[in] an array which holds the packed state vector, must be
     *         at least the size returned by getStateVector.
     *
     * @return the number of items copied from the state vector, negative
     *         on failure.
     */
    virtual int setStateVector(const double *stateVector);

    virtual void convertToConcentrations();
    virtual void updateDependentSpeciesValues();
    virtual void computeAllRatesOfChange();


    /**
     * where most of the juicy bits occur.
     *
     * the state vector y is the rate rule values and floating species
     * concentrations concatenated. y is of length numFloatingSpecies + numRateRules.
     *
     * The state vector is packed such that the first n raterule elements are the
     * values of the rate rules, and the last n floatingspecies are the floating
     * species values.
     *
     * @param[in] time current simulator time
     * @param[in] y state vector, must be of size returned by getStateVector
     * @param[out] dydt calculated rate of change of the state vector, if null,
     * it is ignored.
     */
    virtual void evalModel(double time, const double *y, double* dydt=0);

    virtual void evalEvents(const double time, const double *y);

    virtual void resetEvents();
    virtual void testConstraints();

    virtual string getInfo();

    virtual int getFloatingSpeciesIndex(const string&);
    virtual string getFloatingSpeciesName(int);
    virtual int getBoundarySpeciesIndex(const string&);
    virtual string getBoundarySpeciesName(int);
    virtual int getBoundarySpeciesCompartmentIndex(int);

    /**
     * get the floating species amounts
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[out] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int getFloatingSpeciesAmounts(int len, int const *indx,
            double *values);

    /**
     * get the floating species concentrations
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[out] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int getFloatingSpeciesConcentrations(int len, int const *indx,
            double *values);


    /**
     * set the floating species concentrations
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[in] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int setFloatingSpeciesConcentrations(int len, int const *indx,
            double const *values);


    /**
     * get the boundary species amounts
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[out] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int getBoundarySpeciesAmounts(int len, int const *indx,
            double *values);

    /**
     * get the boundary species concentrations
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[out] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int getBoundarySpeciesConcentrations(int len, int const *indx,
            double *values);

    /**
     * get the boundary species concentrations
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of boundary species to return.
     * @param[in] values an array of at least length len which will store the
     *                returned boundary species amounts.
     */
    virtual int setBoundarySpeciesConcentrations(int len, int const *indx,
            double const *values);


    virtual int getGlobalParameterIndex(const string&);
    virtual string getGlobalParameterName(int);
    virtual int getCompartmentIndex(const string&);
    virtual string getCompartmentName(int);
    virtual int getReactionIndex(const string&);
    virtual string getReactionName(int);

    virtual void print(std::ostream &stream);

    /**
     * get the event time delays
     *
     * @param[in] len the length of the indx and values arrays.
     * @param[in] indx an array of length len of event indices
     * @param[out] values an array of at least length len which will store the
     *                event delays.
     */
    virtual int getEventDelays(int len, int const *indx, double *values);

    virtual int getEventPriorities(int len, int const *indx, double *values);

    virtual void eventAssignment(int eventId);

    virtual double* evalEventAssignment(int eventId);

    virtual void applyEventAssignment(int eventId, double *values);

    /**
     * get the event status, false if the even is not triggered, true if it is.
     */
    virtual int getEventStatus(int len, const int *indx, unsigned char *values);


    virtual const SymbolList &getConservations();

    virtual const StringList getConservationNames();

    /**
     * using the current model state, evaluate and store all the reaction rates.
     */
    virtual void evalReactionRates();

    virtual int applyPendingEvents(const double *stateVector, double timeEnd,
            double tout);

    virtual void evalEvents(double timeEnd, const unsigned char* previousEventStatus,
            const double *initialState, double* finalState);

    virtual void evalEventRoots(double time, const double *stateVector, const double* y,
            double* gdot);

    virtual double getNextPendingEventTime(bool pop);

    virtual int getPendingEventSize();

    static LLVMExecutableModel* dummy();

private:
    ModelData modelData;
    ModelData modelDataCopy;
    LLVMModelDataSymbols *symbols;
    llvm::LLVMContext *context;
    llvm::ExecutionEngine *executionEngine;
    std::string *errStr;
    int stackDepth;


    LLVMEvalInitialConditionsCodeGen::FunctionPtr evalInitialConditionsPtr;
    LLVMEvalReactionRatesCodeGen::FunctionPtr evalReactionRatesPtr;
    LLVMGetBoundarySpeciesAmountCodeGen::FunctionPtr getBoundarySpeciesAmountPtr;
    LLVMGetFloatingSpeciesAmountCodeGen::FunctionPtr getFloatingSpeciesAmountPtr;
    LLVMGetBoundarySpeciesConcentrationCodeGen::FunctionPtr getBoundarySpeciesConcentrationPtr;
    LLVMGetFloatingSpeciesConcentrationCodeGen::FunctionPtr getFloatingSpeciesConcentrationPtr;
    LLVMGetCompartmentVolumeCodeGen::FunctionPtr getCompartmentVolumePtr;
    LLVMGetGlobalParameterCodeGen::FunctionPtr getGlobalParameterPtr;
    LLVMEvalRateRuleRatesCodeGen::FunctionPtr evalRateRuleRatesPtr;

    double getFloatingSpeciesConcentration(int index);

    friend class LLVMModelGenerator;
};

} /* namespace rr */
#endif /* rrLLVMExecutableModelH */
