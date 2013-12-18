/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.4
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace libsbml {

 using System;
 using System.Runtime.InteropServices;

/** 
 * @sbmlpackage{core}
 *
@htmlinclude pkg-marker-core.html SBML converter for replacing initial assignments.
 *
 * @htmlinclude libsbml-facility-only-warning.html
 *
 * This is an SBML converter for replacing InitialAssignment objects 
 * (when possible) by setting the initial value attributes on the model
 * objects being assigned.  In other words, for every object that is
 * the target of an initial assignment in the model, it evaluates the
 * mathematical expression of the assignment to get a numerical value,
 * and then sets the corresponding attribute of the object to the
 * value.  The effects for different kinds of SBML components are
 * as follows:
 * 
 * <center>
 * <table border='0' class='text-table width80 normal-font alt-row-colors'>
 *  <tr style='background: lightgray; font-size: 14px;'>
 *      <th align='left' width='200'>Component</th>
 *      <th align='left'>Effect</th>
 *  </tr>
 *  <tr>
 *  <td>Compartment</td>
 *  <td>Sets the value of the <code>size</code> attribute.</td>
 *  </tr>
 *  <tr>
 *  <td>Species</td>
 *  <td>Sets the value of either the <code>initialAmount</code>
 *  or the <code>initialConcentration</code> attributes, depending
 *  on the value of the Species object's
 *  <code>hasOnlySubstanceUnits</code> attribute.</td>
 *  </tr>
 *  <tr>
 *  <td>Parameter</td>
 *  <td>Sets the value of the <code>value</code> attribute.</td>
 *  </tr>
 *  <tr>
 *  <td>SpeciesReference</td>
 *  <td>Sets the value of the <code>stoichiometry</code> attribute
 *  in the Reaction object where the SpeciesReference object appears.</td>
 *  </tr>
 * </table>
 * </center>
 *
 * @see SBMLFunctionDefinitionConverter
 * @see SBMLLevelVersionConverter
 * @see SBMLRuleConverter
 * @see SBMLStripPackageConverter
 * @see SBMLUnitsConverter
 */

public class SBMLInitialAssignmentConverter : SBMLConverter {
	private HandleRef swigCPtr;
	
	internal SBMLInitialAssignmentConverter(IntPtr cPtr, bool cMemoryOwn) : base(libsbmlPINVOKE.SBMLInitialAssignmentConverter_SWIGUpcast(cPtr), cMemoryOwn)
	{
		//super(libsbmlPINVOKE.SBMLInitialAssignmentConverterUpcast(cPtr), cMemoryOwn);
		swigCPtr = new HandleRef(this, cPtr);
	}
	
	internal static HandleRef getCPtr(SBMLInitialAssignmentConverter obj)
	{
		return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
	}
	
	internal static HandleRef getCPtrAndDisown (SBMLInitialAssignmentConverter obj)
	{
		HandleRef ptr = new HandleRef(null, IntPtr.Zero);
		
		if (obj != null)
		{
			ptr             = obj.swigCPtr;
			obj.swigCMemOwn = false;
		}
		
		return ptr;
	}

  ~SBMLInitialAssignmentConverter() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          libsbmlPINVOKE.delete_SBMLInitialAssignmentConverter(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static void init() {
    libsbmlPINVOKE.SBMLInitialAssignmentConverter_init();
  }

  
/**
   * Creates a new SBMLInitialAssignmentConverter object.
   */ public
 SBMLInitialAssignmentConverter() : this(libsbmlPINVOKE.new_SBMLInitialAssignmentConverter__SWIG_0(), true) {
  }

  
/**
   * Copy constructor; creates a copy of an SBMLInitialAssignmentConverter
   * object.
   *
   * @param obj the SBMLInitialAssignmentConverter object to copy.
   */ public
 SBMLInitialAssignmentConverter(SBMLInitialAssignmentConverter obj) : this(libsbmlPINVOKE.new_SBMLInitialAssignmentConverter__SWIG_1(SBMLInitialAssignmentConverter.getCPtr(obj)), true) {
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
  }

  
/**
   * Creates and returns a deep copy of this SBMLInitialAssignmentConverter
   * object.
   * 
   * @return a (deep) copy of this converter.
   */ public
 SBMLConverter clone() {
    IntPtr cPtr = libsbmlPINVOKE.SBMLInitialAssignmentConverter_clone(swigCPtr);
    SBMLConverter ret = (cPtr == IntPtr.Zero) ? null : new SBMLConverter(cPtr, true);
    return ret;
  }

  
/**
   * Returns @c true if this converter object's properties match the given
   * properties.
   *
   * A typical use of this method involves creating a ConversionProperties
   * object, setting the options desired, and then calling this method on
   * an SBMLInitialAssignmentConverter object to find out if the object's
   * property values match the given ones.  This method is also used by
   * SBMLConverterRegistry::getConverterFor(@if java ConversionProperties props@endif)
   * to search across all registered converters for one matching particular
   * properties.
   * 
   * @param props the properties to match.
   * 
   * @return @c true if this converter's properties match, @c false
   * otherwise.
   */ public
 bool matchesProperties(ConversionProperties props) {
    bool ret = libsbmlPINVOKE.SBMLInitialAssignmentConverter_matchesProperties(swigCPtr, ConversionProperties.getCPtr(props));
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  
/** 
   * Perform the conversion.
   *
   * This method causes the converter to do the actual conversion work,
   * that is, to convert the SBMLDocument object set by
   * SBMLConverter::setDocument(@if java SBMLDocument doc@endif) and
   * with the configuration options set by
   * SBMLConverter::setProperties(@if java ConversionProperties props@endif).
   * 
   * @return  integer value indicating the success/failure of the operation.
   * @if clike The value is drawn from the enumeration
   * #OperationReturnValues_t. @endif The possible values are:
   * @li @link libsbmlcs.libsbml.LIBSBML_OPERATION_SUCCESS LIBSBML_OPERATION_SUCCESS @endlink
   * @li @link libsbmlcs.libsbml.LIBSBML_OPERATION_FAILED LIBSBML_OPERATION_FAILED @endlink
   * @li @link libsbmlcs.libsbml.LIBSBML_INVALID_OBJECT LIBSBML_INVALID_OBJECT @endlink
   */ public
 int convert() {
    int ret = libsbmlPINVOKE.SBMLInitialAssignmentConverter_convert(swigCPtr);
    return ret;
  }

  
/**
   * Returns the default properties of this converter.
   * 
   * A given converter exposes one or more properties that can be adjusted
   * in order to influence the behavior of the converter.  This method
   * returns the @em default property settings for this converter.  It is
   * meant to be called in order to discover all the settings for the
   * converter object.
   *
   * @return the ConversionProperties object describing the default properties
   * for this converter.
   */ public
 ConversionProperties getDefaultProperties() {
    ConversionProperties ret = new ConversionProperties(libsbmlPINVOKE.SBMLInitialAssignmentConverter_getDefaultProperties(swigCPtr), true);
    return ret;
  }

}

}
