//-----------------------------------------------------------------------------
// Created on: 03 February 2016
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

#ifndef xde_STEP_h
#define xde_STEP_h

// A-Situs includes
#include <analysis_situs.h>

// A-Situs (common) includes
#include <common_cad_properties.h>

// A-Situs (XDE) includes
#include <xde_model.h>

// OCCT includes
#include <TCollection_AsciiString.hxx>
#include <XSControl_WorkSession.hxx>

//! STEP-XDE interoperability tool.
class xde_STEP
{
public:

  static bool Read(const TCollection_AsciiString& filename,
                   Handle(xde_model)&             result_model,
                   common_cad_properties&         result_file_props);

protected:

  static void
    clearSession(const Handle(XSControl_WorkSession)& WS);

private:

  xde_STEP() {}
  ~xde_STEP() {}

};

#endif
