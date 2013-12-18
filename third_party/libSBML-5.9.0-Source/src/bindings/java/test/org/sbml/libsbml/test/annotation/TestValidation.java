/*
 * @file    TestValidation.java
 * @brief   Validation of Date ModelCreator and ModelHistory unit tests
 *
 * @author  Akiya Jouraku (Java conversion)
 * @author  Sarah Keating 
 * 
 * ====== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING ======
 *
 * DO NOT EDIT THIS FILE.
 *
 * This file was generated automatically by converting the file located at
 * src/annotation/test/TestValidation.cpp
 * using the conversion program dev/utilities/translateTests/translateTests.pl.
 * Any changes made here will be lost the next time the file is regenerated.
 *
 * -----------------------------------------------------------------------------
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 *
 * Copyright 2005-2010 California Institute of Technology.
 * Copyright 2002-2005 California Institute of Technology and
 *                     Japan Science and Technology Corporation.
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.  A copy of the license agreement is provided
 * in the file named "LICENSE.txt" included with this software distribution
 * and also available online as http://sbml.org/software/libsbml/license.html
 * -----------------------------------------------------------------------------
 */

package org.sbml.libsbml.test.annotation;

import org.sbml.libsbml.*;

import java.io.File;
import java.lang.AssertionError;

public class TestValidation {

  static void assertTrue(boolean condition) throws AssertionError
  {
    if (condition == true)
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertEquals(Object a, Object b) throws AssertionError
  {
    if ( (a == null) && (b == null) )
    {
      return;
    }
    else if ( (a == null) || (b == null) )
    {
      throw new AssertionError();
    }
    else if (a.equals(b))
    {
      return;
    }

    throw new AssertionError();
  }

  static void assertNotEquals(Object a, Object b) throws AssertionError
  {
    if ( (a == null) && (b == null) )
    {
      throw new AssertionError();
    }
    else if ( (a == null) || (b == null) )
    {
      return;
    }
    else if (a.equals(b))
    {
      throw new AssertionError();
    }
  }

  static void assertEquals(boolean a, boolean b) throws AssertionError
  {
    if ( a == b )
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertNotEquals(boolean a, boolean b) throws AssertionError
  {
    if ( a != b )
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertEquals(int a, int b) throws AssertionError
  {
    if ( a == b )
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertNotEquals(int a, int b) throws AssertionError
  {
    if ( a != b )
    {
      return;
    }
    throw new AssertionError();
  }

  public void test_Validation_CVTerm1()
  {
    CVTerm cv = new CVTerm();
    assertTrue( cv != null );
    assertEquals( false, (cv.hasRequiredAttributes()) );
    cv.setQualifierType(libsbml.MODEL_QUALIFIER);
    assertEquals( false, (cv.hasRequiredAttributes()) );
    cv.setModelQualifierType(libsbml.BQM_IS);
    assertEquals( false, (cv.hasRequiredAttributes()) );
    cv.addResource("ggg");
    assertEquals( true, (cv.hasRequiredAttributes()) );
    cv = null;
  }

  public void test_Validation_CVTerm2()
  {
    CVTerm cv = new CVTerm();
    assertTrue( cv != null );
    assertEquals( false, (cv.hasRequiredAttributes()) );
    cv.setQualifierType(libsbml.BIOLOGICAL_QUALIFIER);
    assertEquals( false, (cv.hasRequiredAttributes()) );
    cv.setBiologicalQualifierType(libsbml.BQB_IS);
    assertEquals( false, (cv.hasRequiredAttributes()) );
    cv.addResource("ggg");
    assertEquals( true, (cv.hasRequiredAttributes()) );
    cv = null;
  }

  public void test_Validation_Date1()
  {
    Date date = new Date(200,12,30,12,15,45,1,2,0);
    assertTrue( date != null );
    assertEquals( false, (date.representsValidDate()) );
    date = null;
  }

  public void test_Validation_Date2()
  {
    Date date = new Date(2007,14,30,12,15,45,1,2,0);
    assertTrue( date != null );
    assertEquals( false, (date.representsValidDate()) );
    date = null;
  }

  public void test_Validation_Date3()
  {
    Date date = new Date("Jan 12");
    assertTrue( date != null );
    assertEquals( false, (date.representsValidDate()) );
    date = null;
  }

  public void test_Validation_Date4()
  {
    Date date = new Date(2007,12,30,12,15,45,1,2,0);
    assertTrue( date != null );
    assertEquals( true, date.representsValidDate() );
    date = null;
  }

  public void test_Validation_ModelCreator()
  {
    ModelCreator mc = new ModelCreator();
    assertTrue( mc != null );
    assertEquals( false, (mc.hasRequiredAttributes()) );
    mc.setEmail("k123");
    assertEquals( false, (mc.hasRequiredAttributes()) );
    mc.setFamilyName("Keating");
    assertEquals( false, (mc.hasRequiredAttributes()) );
    mc.setGivenName("Sarah");
    assertEquals( true, mc.hasRequiredAttributes() );
    mc = null;
  }

  public void test_Validation_ModelHistory1()
  {
    ModelHistory mh = new ModelHistory();
    assertTrue( mh != null );
    assertEquals( false, (mh.hasRequiredAttributes()) );
    Date date = new Date(2007,12,30,12,15,45,1,2,0);
    mh.setCreatedDate(date);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    mh.setModifiedDate(date);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    ModelCreator mc = new ModelCreator();
    mc.setFamilyName("Keating");
    mc.setGivenName("Sarah");
    mh.addCreator(mc);
    assertEquals( true, mh.hasRequiredAttributes() );
    mh = null;
  }

  public void test_Validation_ModelHistory2()
  {
    ModelHistory mh = new ModelHistory();
    assertTrue( mh != null );
    assertEquals( false, (mh.hasRequiredAttributes()) );
    Date date = new Date(200,12,30,12,15,45,1,2,0);
    mh.setCreatedDate(date);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    mh.setModifiedDate(date);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    ModelCreator mc = new ModelCreator();
    mc.setFamilyName("Keating");
    mc.setGivenName("Sarah");
    mh.addCreator(mc);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    mh = null;
  }

  public void test_Validation_ModelHistory3()
  {
    ModelHistory mh = new ModelHistory();
    assertTrue( mh != null );
    assertEquals( false, (mh.hasRequiredAttributes()) );
    Date date = new Date(2007,12,30,12,15,45,1,2,0);
    mh.setCreatedDate(date);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    mh.setModifiedDate(date);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    ModelCreator mc = new ModelCreator();
    mc.setFamilyName("Keating");
    mh.addCreator(mc);
    assertEquals( false, (mh.hasRequiredAttributes()) );
    mh = null;
  }

  /**
   * Loads the SWIG-generated libSBML Java module when this class is
   * loaded, or reports a sensible diagnostic message about why it failed.
   */
  static
  {
    String varname;
    String shlibname;

    if (System.getProperty("mrj.version") != null)
    {
      varname = "DYLD_LIBRARY_PATH";    // We're on a Mac.
      shlibname = "libsbmlj.jnilib and/or libsbml.dylib";
    }
    else
    {
      varname = "LD_LIBRARY_PATH";      // We're not on a Mac.
      shlibname = "libsbmlj.so and/or libsbml.so";
    }

    try
    {
      System.loadLibrary("sbmlj");
      // For extra safety, check that the jar file is in the classpath.
      Class.forName("org.sbml.libsbml.libsbml");
    }
    catch (SecurityException e)
    {
      e.printStackTrace();
      System.err.println("Could not load the libSBML library files due to a"+
                         " security exception.\n");
      System.exit(1);
    }
    catch (UnsatisfiedLinkError e)
    {
      e.printStackTrace();
      System.err.println("Error: could not link with the libSBML library files."+
                         " It is likely\nyour " + varname +
                         " environment variable does not include the directories\n"+
                         "containing the " + shlibname + " library files.\n");
      System.exit(1);
    }
    catch (ClassNotFoundException e)
    {
      e.printStackTrace();
      System.err.println("Error: unable to load the file libsbmlj.jar."+
                         " It is likely\nyour -classpath option and CLASSPATH" +
                         " environment variable\n"+
                         "do not include the path to libsbmlj.jar.\n");
      System.exit(1);
    }
  }
}

