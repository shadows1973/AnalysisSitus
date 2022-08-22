//-----------------------------------------------------------------------------
// Created on: 11 December(*) 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef asiVisu_IVCurve2dDataProvider_h
#define asiVisu_IVCurve2dDataProvider_h

// asiVisu includes
#include <asiVisu_CurveDataProvider.h>

//! Data provider for a single two-dimensional curve in IV.
class asiVisu_IVCurve2dDataProvider : public asiVisu_CurveDataProvider
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_IVCurve2dDataProvider, asiVisu_CurveDataProvider)

public:

  //! Constructor.
  //! \param N [in] source Node.
  asiVisu_EXPORT
    asiVisu_IVCurve2dDataProvider(const Handle(ActAPI_INode)& N);

public:

  //! \return curve type.
  asiVisu_EXPORT virtual Handle(Standard_Type)
    GetCurveType() const;

  //! Accessor for curve.
  //! \param f [out] first parameter.
  //! \param l [out] last parameter.
  //! \return curve.
  asiVisu_EXPORT virtual Handle(Geom2d_Curve)
    GetCurve2d(double& f, double& l) const;

  //! Not used.
  asiVisu_EXPORT virtual Handle(Geom_Curve)
    GetCurve(double& f, double& l) const;

  //! \return Boolean flag indicating whether to draw orientation tip.
  asiVisu_EXPORT virtual bool
    GetDrawOrientationTip() const;

  //! Returns ID of the Data Node which is being sourced by the visualization
  //! pipeline. This ID is bound to the pipeline's actor in order to have a
  //! back-reference from Presentation to Data Object.
  //! \return Node ID.
  asiVisu_EXPORT virtual ActAPI_DataObjectId
    GetNodeID() const;

public:

  //! Creates a copy of the Data Provider.
  //! \return copy.
  asiVisu_EXPORT Handle(asiVisu_IVCurve2dDataProvider)
    Clone() const;

private:

  //! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
  //! translation process.
  //! \return source Parameters.
  virtual Handle(ActAPI_HParameterList)
    translationSources() const;

private:

  Handle(ActAPI_INode) m_node; //!< Source Node.

};

#endif
