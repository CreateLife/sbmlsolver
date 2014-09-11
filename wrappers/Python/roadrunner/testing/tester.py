# Test Module for RoadRunner
#
# Usage:
# import rrTester
# runTester (pathtoModelFile, modelFileName)


#-------------------------------------------------------------
# Tests for steady state and stoichiometric calculations in
# roadRunner. Herbert M Sauro November 2012
# Nov 2013: Modified to test Andy's SWIG API
#-------------------------------------------------------------

#------------------------------------------
# Change this line for different test files
#nameOfResultsFile = 'results_roadRunnerTest_1.txt'


import random
import string
import roadrunner
import re
from numpy import *


# Module wide file handle
fHandle = ''
rpadding = 45
sbmlStr = ''
JarnacStr = ''

# --------------------------------------------------------------------------
# SUPPORT ROUTINES
# --------------------------------------------------------------------------

def expectApproximately (a, b, tol):
    diff = a - b
    return abs(diff) < tol

def passMsg (errorFlag, msg = ""):
    if msg:
        msg = "\n    " + msg
    if not errorFlag:
        return "PASS"
    return "*****FAIL*****" + msg

# Empty lines are ignored
# Lines starting with # are also ignored

def readLine ():
    line = fHandle.readline()

    while (line == '') or (line[0] == '#') or (line[0] == '\n'):
        if line == '':
            return line
        line = fHandle.readline();
    return line.strip('\n')

def divide(line):
    words = line.strip()
    words = words.strip('"')
    words = re.compile("[ =]").split(words)
    while "" in words: words.remove("")
    return words

def jumpToNextTest():
    line = readLine()
    #line = ''
    #while line == '':
    #      line = fHandle.readline().strip ('\n')
    while (line != "") and (line[0] != '['):
        line = readLine()
    return line

def getSBMLStr ():
    sbmlStr = ''
    line = fHandle.readline()
    while (line[0] != '['):
        sbmlStr = sbmlStr + line
        line = fHandle.readline()
    return sbmlStr, line.strip()

def getJarnacStr ():
    JarnacStr = ''
    line = fHandle.readline()
    while (line != '[END_MODEL]' + '\n'):
        JarnacStr = JarnacStr + line
        line = fHandle.readline()
    return JarnacStr

# ------------------------------------------------------------------------
# TESTS START HERE
# ------------------------------------------------------------------------
def setConservationLaw(rrInstance, testId):
    line = readLine ()
    if line == 'True':
        rrInstance.conservedMoietyAnalysis = True
    else:
        rrInstance.conservedMoietyAnalysis = False

def checkSpeciesConcentrations(rrInstance, testId):
    words = []
    species = []
    m = rrInstance.model.getNumFloatingSpecies()
    for i in range (0,m):
        words = divide(readLine())
        words.append (rrInstance.model[words[0]])
        species.append (words)

    # Steady State Concentrations
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    for i in range (0,m):
        expectedValue =  float (species[i][1])
        if expectApproximately (expectedValue, species[i][2], 1E-6) == False:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkFluxes(rrInstance, testId):
    words = []
    fluxes = []
    # Steady State Fluxes
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    n = rrInstance.model.getNumReactions();
    for i in range (0,n):
        line = readLine ()
        words = line.split("=")
        words.append (rrInstance.model[words[0]])
        fluxes.append (words)

    for i in range (0,n):
        expectedValue = float (fluxes[i][1])
        if expectApproximately (expectedValue, fluxes[i][2], 1E-6) == False:
            errorFlag = True
            break
    print passMsg (errorFlag)

def checkFullJacobian(rrInstance, testId):
    # Jacobian
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    Jacobian = rrInstance.getFullJacobian()
    for i in range(0,m):
        line = readLine ()
        words = line.split()
        for j in range(0,m):
            expectedValue = float(words[j])
            if expectApproximately (expectedValue, Jacobian[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)


def checkIndividualEigenvalues(rrInstance, testId):
    # Eigenvalues
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    try:
        for i in range(0,m):
            words = divide(readLine())
            eigenvalueName = words[0]
            realPart = rrInstance.getValue ('eigen(' + eigenvalueName + ')')
            realPart = float (realPart)

            if expectApproximately (realPart, float(words[1]), 1E-5) == False:
                errorFlag = True
                break
        print passMsg (errorFlag)
    except Exception, e:
        print('Unexpected error in checkIndividualEigenvalues:' + str(e))


def checkEigenvalueMatrix(rrInstance, testId):
    # Eigenvalues
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    eigenvalues = rrInstance.getEigenvalues()
    for i in range(0,m):
        line = readLine ()
        words = line.split()
        realPart = float (words[0])
        # Check if there is an imaginary part
        if len (words) == 1:
            imagPart = 0
        else:
            imagPart= float (words[1])
        if (expectApproximately (realPart, eigenvalues[i,0], 1E-5) == False) or (expectApproximately (imagPart, eigenvalues[i,1], 1E-6)) == False:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkStoichiometryMatrix(rrInstance, testId):
    # Stoichiometry matrix
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    n = rrInstance.model.getNumReactions();
    st = rrInstance.model.getCurrentStoichiometryMatrix()
    for i in range(0,m):
        line = readLine ()
        words = line.split()
        for j in range(0,n):
            if expectApproximately(float (words[j]), st[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)

def checkLinkMatrix(rrInstance, testId):
    # Link matrix
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    st = rrInstance.getLinkMatrix()
    for i in range(0,m):
        words = readLine ().split()
        for j in range(0,m):
            if expectApproximately(float (words[j]), st[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)

def checkUnscaledConcentrationControlMatrix(rrInstance, testId):
    # Unscaled Concentration Control matrix
    print string.ljust ("Check " + testId, rpadding),
    words = []
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    n = rrInstance.model.getNumReactions();
    st = rrInstance.getUnscaledConcentrationControlCoefficientMatrix();
    for i in range(0,m):
        words = readLine ().split()
        for j in range(0,n):
            if expectApproximately(float (words[j]), st[i,j], 1E-5) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)


def checkScaledConcentrationControlMatrix(rrInstance, testId):
    # Unscaled Concentration Control matrix
    print string.ljust ("Check " + testId, rpadding),
    words = []
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    n = rrInstance.model.getNumReactions();
    st = rrInstance.getScaledConcentrationControlCoefficientMatrix();
    for i in range(0,m):
        words = readLine ().split()
        for j in range(0,n):
            if expectApproximately(float (words[j]), st[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)


def checkUnscaledFluxControlCoefficientMatrix(rrInstance, testId):
    # Unscaled Flux Control matrix
    print string.ljust ("Check " + testId, rpadding),
    words = []
    errorFlag = False
    n = rrInstance.model.getNumReactions();
    st = rrInstance.getUnscaledFluxControlCoefficientMatrix();
    for i in range(0,n):
        words = readLine ().split()
        for j in range(0,n):
            if expectApproximately(float (words[j]), st[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)


def checkScaledFluxControlCoefficientMatrix(rrInstance, testId):
    # Unscaled Flux Control matrix
    print string.ljust ("Check " + testId, rpadding),
    words = []
    errorFlag = False
    n = rrInstance.model.getNumReactions();
    st = rrInstance.getScaledFluxControlCoefficientMatrix()
    for i in range(0,n):
        words = readLine ().split()
        for j in range(0,n):
            if expectApproximately(float (words[j]), st[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)


def checkUnscaledElasticityMatrix(rrInstance, testId):
    # Jacobian
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    uee = rrInstance.getUnscaledElasticityMatrix()
    for i in range(0,m):
        line = readLine ()
        words = line.split()
        for j in range(0,m):
            expectedValue = float(words[j])
            if expectApproximately (expectedValue, uee[i,j], 1E-6) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)

def checkScaledElasticityMatrix(rrInstance, testId):
    # Jacobian
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    m = rrInstance.model.getNumFloatingSpecies()
    ee = rrInstance.getScaledElasticityMatrix()
    for i in range(0,m):
        line = readLine ()
        words = line.split()
        for j in range(0,m):
            expectedValue = float(words[j])
            if expectApproximately (expectedValue, ee[i,j], 1E-5) == False:
                errorFlag = True
                break
    print passMsg (errorFlag)


def checkGetFloatingSpeciesIds(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    line = readLine ()
    words = line.split()
    expected = rrInstance.model.getFloatingSpeciesIds()
    m = rrInstance.model.getNumFloatingSpecies()
    for i in range(0,m):
        if words[i] != expected[i]:
            errorFlag = True
            break
    print passMsg (errorFlag)

def checkGetBoundarySpeciesIds(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    line = readLine ()
    words = line.split()
    expected = rrInstance.model.getBoundarySpeciesIds()
    m = rrInstance.model.getNumBoundarySpecies()
    for i in range(0,m):
        if words[i] != expected[i]:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkGetGlobalParameterIds (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    line = readLine ()
    words = line.split()
    expected = rrInstance.model.getGlobalParameterIds()
    m = rrInstance.model.getNumGlobalParameters()
    for i in range(0,m):
        if words[i] != expected[i]:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkGetCompartmentIds (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    line = readLine ()
    words = line.split()
    expected = rrInstance.model.getCompartmentIds()
    m = rrInstance.model.getNumCompartments()
    for i in range(0,m):
        if words[i] != expected[i]:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkReactionIds (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = divide(readLine())
    expected = rrInstance.model.getReactionIds()
    m = rrInstance.model.getNumReactions();
    for i in range(0,m):
        if words[i] != expected[i]:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkFloatingSpeciesInitialConditionIds (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),

    words = divide(readLine())

    expected = rrInstance.model.getFloatingSpeciesInitConcentrationIds()


    print passMsg (words != expected)


def checkEigenValueIds (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = divide(readLine())
    expected = rrInstance.getEigenvalueIds()
    m = rrInstance.model.getNumFloatingSpecies()
    for i in range(0,m):
        if words[i] != expected[i]:
            errorFlag = True
            break
    print passMsg (errorFlag)


def checkGetRatesOfChangeIds (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    words = divide(readLine())
    expected = rrInstance.model.getRatesOfChangeIds()

    print passMsg (expected != words)


def checkSetSteadyStateSelectionList(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = divide(readLine())
    result = rrInstance.steadyStateSelections = words
    if result == False:
        errorFlag = True
    print passMsg (errorFlag)
    print "Computing Steady State.  Distance to SteadyState:", rrInstance.steadyState()


def checkGetSteadyStateSelectionList(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    words = divide(readLine())
    result = str(rrInstance.steadyStateSelections)

    print passMsg (result == words)


def checkSetTimeCourseSelectionList(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = divide(readLine())
    result = rrInstance.selections = words
    if result == False:
        errorFlag = True
    print passMsg (errorFlag)


def checkGetTimeCourseSelectionList(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    words = divide(readLine())
    result = str(rrInstance.selections)

    print passMsg (result == words)

def compareUpcomingValuesWith(ss, error):
    errorFlag = False
    msg = ""
    words = divide(readLine())
    if (len(words)) == 1:
        for i in range(len(ss)):
            if expectApproximately(float (words[0]), ss[i], error) == False:
                msg = "Expected " + words[0] + ", but calculated " + str(ss[i])
                errorFlag = True
                break;
            if i < len(ss) - 1:
                words = divide(readLine())
    else:
        for i in range (len (ss)):
            if expectApproximately(float (words[i]), ss[i], error) == False:
                msg = "Expected " + words[i] + ", but calculated " + str(ss[i])
                errorFlag = True
                break;
    return errorFlag, msg

def checkComputeSteadyStateValues(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    ss = rrInstance.getSteadyStateValues()
    errorFlag, msg = compareUpcomingValuesWith(ss, 1E-6)
    print passMsg (errorFlag, msg)


def checkFloatingSpeciesConcentrations(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    ss = rrInstance.model.getFloatingSpeciesConcentrations()
    errorFlag, msg = compareUpcomingValuesWith(ss, 1E-6)
    print passMsg (errorFlag, msg)


def checkBoundarySpeciesConcentrations(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    ss = rrInstance.model.getBoundarySpeciesConcentrations()
    errorFlag, msg = compareUpcomingValuesWith(ss, 1E-6)
    print passMsg (errorFlag, msg)


def checkGlobalParameterValues(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    ss = rrInstance.model.getGlobalParameterValues()
    errorFlag, msg = compareUpcomingValuesWith(ss, 1E-6)
    print passMsg (errorFlag, msg)


def checkInitalFloatingSpeciesConcentations(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    ss = rrInstance.model.getFloatingSpeciesInitConcentrations()

    words = readLine().split()

    same = len(words) == len(ss) and \
        len(words) == sum([1 for i,j in zip(words,ss) if expectApproximately(float (i), j, 1E-6)])

    print passMsg (not same)


def checkReactionRates(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    ss = rrInstance.model.getReactionRates()
    errorFlag, msg = compareUpcomingValuesWith(ss, 1E-6)
    print passMsg (errorFlag, msg)


def checkGetReactionRatesByIndex(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = readLine().split()
    n = rrInstance.model.getNumReactions()
    for i in range (n):
        value = rrInstance.model.getReactionRates()[i]
        if expectApproximately(float (words[i]), value, 1E-6) == False:
            errorFlag = True
            break;
    print passMsg (errorFlag)

def checkGetSpeciesInitialConcentrationsByIndex(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = divide(readLine())
    n = len(words) #rrInstance.model.getNumSpecies()
    #for i in range (n):
    #    value = rrInstance.model.getFloatingSpeciesInitialConcentrations()[i]
    #    if expectApproximately(float (words[i]), value, 1E-6) == False:
    #        errorFlag = True
    #        break;
    print passMsg (errorFlag)


def checkNumberOfDependentSpecies(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    value = int (readLine())
    n = rrInstance.model.getNumDepFloatingSpecies()
    if n != value:
        errorFlag = True
    print passMsg (errorFlag)


def checkNumberOfIndependentSpecies(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    value = int (readLine())
    n = rrInstance.model.getNumIndFloatingSpecies()
    if n != value:
        errorFlag = True
    print passMsg (errorFlag)


def checkNumberOfRules(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    value = int (readLine())
    if rrInstance.getNumRules() != value:
        errorFlag = True
    print passMsg (errorFlag)


def checkGetRatesOfChange(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    words = readLine().split()
    values = rrInstance.model.getRatesOfChange()
    for i in range (len(words)):
        if expectApproximately (float (words[i]), values[i], 1E-6) == False:
            errorFlag = True
    print passMsg (errorFlag)


def checkGetReactionRatesEx (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    inputConcs = asarray (divide(readLine()).remove("indexes"), dtype=float64)
    values = rrInstance.getReactionRatesEx (inputConcs)
    outputRates = asarray (divide(readLine()).remove("rates"), dtype=float64)
    if not allclose (values, outputRates):
        errorFlag = True
    print passMsg (errorFlag)


def checkGetRatesOfChangeEx (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    inputConcs = asarray (divide(readLine()).remove("indexes"), dtype=float64)
    values = rrInstance.model.getRatesOfChangeEx (inputConcs)
    outputRates = asarray (divide(readLine()).remove("rates"), dtype=float64)
    if not allclose (values, outputRates):
        errorFlag = True
    print passMsg (errorFlag)


def checkGetRateOfChangeByIndex(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    n = rrInstance.getNumRatesOfChange()
    words = readLine().split()
    for i in range (n):
        value = rrInstance.model.getRatesOfChange()[i]
        if expectApproximately(float (words[i]), value, 1E-6) == False:
            errorFlag = True
            break;
    print passMsg (errorFlag)

# ---------------------------------------------------------------------------

def setGetValues(rrInstance, IdList, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    for i in range (len(IdList)):
        value = random.random()*10
        rrInstance.model[dList[i]] = value
        if expectApproximately (rrInstance.model[IdList[i]], value, 1E-6) == False:
            errorFlag = True
            break
    print passMsg (errorFlag)


def setGetTimeStart(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    value = random.random ()*10
    rrInstance.setTimeStart (value)
    if expectApproximately (rrInstance.getTimeStart (), value, 1E-6) == False:
        errorFlag = True
    print passMsg (errorFlag)


def setGetTimeEnd(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    value = random.random ()*10
    rrInstance.setTimeEnd (value)
    if expectApproximately (rrInstance.getTimeEnd (), value, 1E-6) == False:
        errorFlag = True
    print passMsg (errorFlag)


def setGetNumberOfPoints(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    value = random.randint (1, 100)
    rrInstance.setNumPoints (value)
    if rrInstance.getNumPoints () != value:
        errorFlag = True
    print passMsg (errorFlag)


def setGetTimeCourseSelectionList(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    myList = rrInstance.getFloatingSpeciesIds()
    newList = list (myList)
    random.shuffle (newList)
    rrInstance.setTimeCourseSelectionList (newList)
    if rrInstance.getTimeCourseSelectionList() != newList:
        errorFlag = True
    print passMsg (errorFlag)


def setGetSteadyStateSelectionList(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    myList = rrInstance.getFloatingSpeciesIds()
    newList = list (myList)
    while newList == myList:
        random.shuffle (newList)
    rrInstance.setSteadyStateSelectionList (newList)
    getList = rrInstance.getSteadyStateSelectionList()
    if getList != newList:
        errorFlag = True
    print passMsg (errorFlag)


def setGetFloatingSpeciesByIndex(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    n = rrInstance.getNumFloatingSpecies()
    for i in range (n):
        value = random.random()*10
        rrInstance.setFloatingSpeciesByIndex (i, value)
        if expectApproximately(rrInstance.getFloatingSpeciesByIndex (i), value, 1E-6) == False:
            errorFlag = True
            break;
    print passMsg (errorFlag)


def setGetBoundarySpeciesByIndex(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    n = rrInstance.getNumBoundarySpecies()
    for i in range (n):
        value = random.random()*10
        rrInstance.setBoundarySpeciesByIndex (i, value)
        if expectApproximately(rrInstance.getBoundarySpeciesByIndex (i), value, 1E-6) == False:
            errorFlag = True
            break;
    print passMsg (errorFlag)


def setGetCompartmentByIndex(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    n = rrInstance.getNumCompartments()
    for i in range (n):
        value = random.random()*10
        rrInstance.setCompartmentByIndex (i, value)
        if expectApproximately(rrInstance.getCompartmentByIndex (i), value, 1E-6) == False:
            errorFlag = True
            break;
    print passMsg (errorFlag)


def setGetGlobalParameterByIndex (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    n = rrInstance.getNumberOfGlobalParameters()
    for i in range (n):
        value = random.random()*10
        rrInstance.setGlobalParameterByIndex (i, value)
        if expectApproximately(rrInstance.getGlobalParameterByIndex (i), value, 1E-6) == False:
            errorFlag = True
            break;
    print passMsg (errorFlag)


def setGetFloatingSpeciesConcentrations (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    getArray = rrInstance.getFloatingSpeciesConcentrations()
    setArray = zeros(len(getArray))
    for i in range(len(getArray)):
        value = random.random()*10
        setArray[i] = value
    rrInstance.setFloatingSpeciesConcentrations (setArray)
    if (setArray != rrInstance.getFloatingSpeciesConcentrations()).all():
        errorFlag = True
    print passMsg (errorFlag)


def setGetBoundarySpeciesConcentrations (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    getArray = rrInstance.getBoundarySpeciesConcentrations()
    setArray = zeros(len(getArray))
    for i in range(len(getArray)):
        value = random.random()*10
        setArray[i] = value
    rrInstance.setBoundarySpeciesConcentrations (rrInstance.PythonArrayTorrVector (setArray))
    if (setArray != rrInstance.getBoundarySpeciesConcentrations()).all():
        errorFlag = True
    print passMsg (errorFlag)


def setGetInitialFloatingSpeciesConcentrations (rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    getArray = rrInstance.getFloatingSpeciesInitialConcentrations ()
    setArray = zeros(len(getArray))
    for i in range(len(getArray)):
        value = random.random()*10
        setArray[i] = value
    rrInstance.setFloatingSpeciesInitialConcentrations (setArray)
    if (setArray != rrInstance.getFloatingSpeciesInitialConcentrations()).all():
        errorFlag = True
    print passMsg (errorFlag)


def setGetReset(rrInstance, testId):
    print string.ljust ("Check " + testId, rpadding),
    errorFlag = False
    values = zeros (rrInstance.getNumberOfFloatingSpecies())
    for i in range (len (values)):
        values[i] = random.random()*10
    initial = rrInstance.getFloatingSpeciesInitialConcentrations()
    rrInstance.setFloatingSpeciesConcentrations (values)
    # Should reset the floats by to the current initial condition
    rrInstance.reset()
    values = rrInstance.getFloatingSpeciesConcentrations()
    if(values != initial).all():
        errorFlag = True
    print passMsg (errorFlag)


def checkJacobian(rrInstance, testId):
    # TODO need to update python 2.x printing
    print string.ljust ("Check " + testId, rpadding),
    from roadrunner import Config
    import numpy as n

    errors = False

    # max difference between reduced and full
    maxDiff = 2e-10

    # save the old value
    saved = Config.getValue(Config.ROADRUNNER_JACOBIAN_MODE)

    # set to amounts mode
    Config.setValue(Config.ROADRUNNER_JACOBIAN_MODE, Config.ROADRUNNER_JACOBIAN_MODE_AMOUNTS)
    full = rrInstance.getFullJacobian()
    reduced = rrInstance.getReducedJacobian()

    m = n.max(n.abs(reduced-full))
    if (m > maxDiff):
        errors = True
        print("Jacobians different in amounts mode, max difference: " + str(m))

    # set to conc mode
    Config.setValue(Config.ROADRUNNER_JACOBIAN_MODE, Config.ROADRUNNER_JACOBIAN_MODE_CONCENTRATIONS)

    full = rrInstance.getFullJacobian()
    reduced = rrInstance.getReducedJacobian()

    m = n.max(n.abs(reduced-full))
    if (m > maxDiff):
        errors = True
        print("Jacobians different in concentrations mode, max difference: " + str(m))

    # restore previous value
    Config.setValue(Config.ROADRUNNER_JACOBIAN_MODE, saved)

    print passMsg(errors)








def scriptTests():
    print
    print "Testing Set and Get Functions"
    print "-----------------------------"
    setGetValues(rrInstance.getFloatingSpeciesIds(), 'Set/Get Value (Floats)')
    setGetValues(rrInstance.getBoundarySpeciesIds(), 'Set/Get Value (Boundary)')
    setGetValues(rrInstance.getGlobalParameterIds(), 'Set/Get Value (Global Parameters)')
    setGetValues(rrInstance.getCompartmentIds(), 'Set/Get Value (Compartments)')
    setGetTimeStart('Set/Get TimeStart')
    setGetTimeEnd ('Set/Get TimeEnd')
    setGetNumberOfPoints ('Set/Get Number Of Points')
    setGetTimeCourseSelectionList ('Set/Get Time Course Selection List')
    setGetSteadyStateSelectionList ('Set/Get Steady State Selection List')
    setGetFloatingSpeciesByIndex ('Set/Get Floating Species by Index')
    setGetBoundarySpeciesByIndex ('Set/Get Boundary Species by Index')
    setGetCompartmentByIndex ('Set/Get Compartment by Index')
    setGetGlobalParameterByIndex ('Set/Get Global Parameter buy Index')
    setGetBoundarySpeciesConcentrations ('Set/Get Boundary Species Concs')
    setGetFloatingSpeciesConcentrations ('Set/Get Floating Species Concs')
    setGetInitialFloatingSpeciesConcentrations ('Set/Get Initial Concs')
    setGetReset ('Set/Get Reset Method')


# ------------------------------------------------------------------------
# List of tests
functions = {'[Amount/Concentration Jacobians]' : checkJacobian,
             '[Boundary Species Concentrations]': checkBoundarySpeciesConcentrations,
             '[Boundary Species Ids]': checkGetBoundarySpeciesIds,
             '[Compartment Ids]': checkGetCompartmentIds,
             '[Compute Steady State Values]': checkComputeSteadyStateValues,
             '[Conservation Laws]': setConservationLaw,
             '[Eigenvalue Matrix]': checkEigenvalueMatrix,
             '[Floating Species Concentrations]': checkFloatingSpeciesConcentrations,
             '[Floating Species Ids]': checkGetFloatingSpeciesIds,
             '[Fluxes]': checkFluxes,
             '[Full Jacobian]': checkFullJacobian,
             '[Get Eigenvalue Ids]': checkEigenValueIds,
             '[Get Global Parameter Values]': checkGlobalParameterValues,
             '[Get Initial Floating Species Concs]': checkInitalFloatingSpeciesConcentations,
#             '[Get Rate of Change by Index]': checkGetRateOfChangeByIndex,
#             '[Get Rates Of Change Ids]': checkGetRatesOfChangeIds,
#             '[Get Rates Of Change]': checkGetRatesOfChange,
#             '[Get Rates of Change Ex]': checkGetRatesOfChangeEx,
             '[Get Reaction Rate By Index]': checkGetReactionRatesByIndex,
#             '[Get Reaction Rates Ex]': checkGetReactionRatesEx,
             '[Get Reaction Rates]': checkReactionRates,
             '[Get Species Initial Concentrations By Index]': checkGetSpeciesInitialConcentrationsByIndex,
             '[Get Steady State Selection List]': checkGetSteadyStateSelectionList,
             '[Get Steady State Selection List 2]': checkGetSteadyStateSelectionList,
             '[Get Time Course Selection List]': checkGetTimeCourseSelectionList,
             '[Global Parameter Ids]': checkGetGlobalParameterIds,
             '[Individual Eigenvalues]': checkIndividualEigenvalues,
             '[Link Matrix]': checkLinkMatrix,
             '[Number of Dependent Species]': checkNumberOfDependentSpecies,
             '[Number of Independent Species]': checkNumberOfIndependentSpecies,
             '[Reaction Ids]': checkReactionIds,
             '[Scaled Concentration Control Matrix]': checkScaledConcentrationControlMatrix,
             '[Scaled Elasticity Matrix]': checkScaledElasticityMatrix,
             '[Scaled Flux Control Matrix]': checkScaledFluxControlCoefficientMatrix,
             '[Set Steady State Selection List]': checkSetSteadyStateSelectionList,
             '[Set Steady State Selection List 2]': checkSetSteadyStateSelectionList,
             '[Set Time Course Selection List]': checkSetTimeCourseSelectionList,
             '[Species Concentrations]': checkSpeciesConcentrations,
             '[Species Initial Condition Ids]': checkFloatingSpeciesInitialConditionIds,
             '[Stoichiometry Matrix]': checkStoichiometryMatrix,
             '[Unscaled Concentration Control Matrix]': checkUnscaledConcentrationControlMatrix,
             '[Unscaled Elasticity Matrix]': checkUnscaledElasticityMatrix,
             '[Unscaled Flux Control Matrix]': checkUnscaledFluxControlCoefficientMatrix,
              }


def getDefaultTestDir():
    import os.path as p
    d = p.dirname(__file__)
    return p.join(d,"test_data")


def runTester (testDir=None):
    """
    Run a series of tests from a testing dir.

    This enumerates all the files in a directory, and all the files ending with '.rrtest'
    are assumed to be testing files.
    """
    global fHandle
    global sbmlStr
    global JarnacStr

    import os.path as p
    from glob import glob

    if testDir is None:
        testDir = getDefaultTestDir()

    if not p.isdir(testDir):
        raise Exception("{} is NOT a directory".format(testDir))

    files = glob(p.join(testDir, "*.rrtest"))

    for file in files:
        print "\n\nStarting Test on ", file

        # set the globals, these should be class instance vars...
        fHandle = open (file, 'r')
        testId = jumpToNextTest()

        if testId == '[SBML]':
            sbmlStr, testId = getSBMLStr ()
        
        if testId == '[JARNAC]':
            JarnacStr = getJarnacStr ()
            testId = jumpToNextTest()

        #print "\n", "Info:"+ "\n"

        # Create a roadRunner instance
        print "Creating roadRunner Instance..."
        rrInstance = roadrunner.RoadRunner()

        #rrInstance.enableLogging()
        #info = rrInstance.getInfo()
        #print info
        #print

        # Load any initialization actions
        if testId == '[INITIALIZATION]':
            testId = jumpToNextTest ()
            while testId != '[END_INITIALIZATION]':
                if functions.has_key(testId):
                    func = functions[testId]
                    func (rrInstance, testId)
                else:
                    print 'No initialization function found for ' + testId
                testId = jumpToNextTest()
            testId = jumpToNextTest()

        # Load the model into RoadRunner
        if rrInstance.load(sbmlStr) == False:
            print 'Failed to load model.\n'
            #print rrInstance.getLastError()
            raise
        else:
            print 'Successfully loaded model.\n'

        # Now start the tests proper
        while testId != '':
            if functions.has_key(testId):
                func = functions[testId]
                func(rrInstance, testId)
            else:
                #getFloatingSpeciesAmountRates
                print string.ljust (testId, rpadding), 'UNKNOWN TEST'
            testId = jumpToNextTest()
