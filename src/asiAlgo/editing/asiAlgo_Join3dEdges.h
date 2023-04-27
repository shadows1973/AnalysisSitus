//-----------------------------------------------------------------------------
// Created on: 19 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023, Elizaveta Krylova
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

#ifndef asiAlgo_Join3dEdges_h
#define asiAlgo_Join3dEdges_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// OCCT includes
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

//-----------------------------------------------------------------------------

//! Utility to join edge chains.
class asiAlgo_Join3dEdges
{
public:

  //! Constructor.
  //! \param[in] masterCAD shape of the part.
  asiAlgo_EXPORT
    asiAlgo_Join3dEdges(const TopoDS_Shape&  masterCAD,
                        ActAPI_ProgressEntry progress = nullptr,
                        ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Perform edge concatenation.
  //! param[in] edges map of edges to concatenate.
  asiAlgo_EXPORT bool
    Perform(const TopTools_IndexedMapOfShape& edges);

public:

  //! \return result shape.
  const TopoDS_Shape& Result() const
  {
    return m_result;
  }

protected:
  
  //! Joins a couple of edges into a single edge.
  //! \param eFirst     [in]  first edge to join.
  //! \param eSecond    [in]  second edge to join.
  //! \param eFirstOri  [in] first edge orientation.
  //! \param eSecondOri [in] second edge orientation.
  //! \param eResult    [out]   result edge to fill gap.
  //! \param isRev1     [in]  flag if the first edge is reversed.
  //! \param isRev2     [in]  flag if the second edge is reversed.
  //! \return true in case of success, false -- otherwise.
  bool joinEdges(const TopoDS_Edge&        eFirst,
                 const TopoDS_Edge&        eSecond,
                 const TopAbs_Orientation& eFirstOri,
                 const TopAbs_Orientation& eSecondOri,
                 TopoDS_Edge&              eResult,
                 bool                      isRev1,
                 bool                      isRev2) const;

protected:

  TopoDS_Shape m_master; //!< Master model.
  TopoDS_Shape m_result; //!< Result.

  mutable ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  mutable ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

#endif
