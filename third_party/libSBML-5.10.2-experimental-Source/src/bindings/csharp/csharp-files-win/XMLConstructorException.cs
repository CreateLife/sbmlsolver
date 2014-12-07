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
@htmlinclude pkg-marker-core.html Exceptions thrown by some libSBML constructors.
 *
 * @htmlinclude not-sbml-warning.html
 *
 * In some situations, constructors for SBML objects may need to indicate
 * to callers that the creation of the object failed.  The failure may be
 * for different reasons, such as an attempt to use invalid parameters or a
 * system condition such as a memory error.  To communicate this to
 * callers, those classes will throw an XMLConstructorException.  @if cpp
 * Callers can use the standard C++ <code>std::exception</code> method
 * <code>what()</code> to extract the diagnostic message stored with the
 * exception.@endif
 * <p>
 * In languages that don't have an exception mechanism (e.g., C), the
 * constructors generally try to return an error code instead of throwing
 * an exception.
 *
 * @see SBMLConstructorException
 */

public class XMLConstructorException : System.ArgumentException, IDisposable {
	private HandleRef swigCPtr;
	protected bool swigCMemOwn;
	
	internal XMLConstructorException(IntPtr cPtr, bool cMemoryOwn)
	{
		swigCMemOwn = cMemoryOwn;
		swigCPtr    = new HandleRef(this, cPtr);
	}
	
	internal static HandleRef getCPtr(XMLConstructorException obj)
	{
		return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
	}
	
	internal static HandleRef getCPtrAndDisown (XMLConstructorException obj)
	{
		HandleRef ptr = new HandleRef(null, IntPtr.Zero);
		
		if (obj != null)
		{
			ptr             = obj.swigCPtr;
			obj.swigCMemOwn = false;
		}
		
		return ptr;
	}

  ~XMLConstructorException() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          libsbmlPINVOKE.delete_XMLConstructorException(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  internal XMLConstructorException(IntPtr cPtr, bool cMemoryOwn, string v) : base(v)
  {
    swigCMemOwn = cMemoryOwn;
    swigCPtr    = new HandleRef(this, cPtr);
  }

  public XMLConstructorException(string v) : 
   this(libsbmlPINVOKE.new_XMLConstructorException(), true, v) 
  {}

  public XMLConstructorException() : this(libsbmlPINVOKE.new_XMLConstructorException(), true) {
  }

}

}