//-----------------------------------------------------------------------------
// Created on: 21 March 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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

#ifndef asiEngine_Part_h
#define asiEngine_Part_h

// asiEngine includes
#include <asiEngine_Base.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_AAGIterator.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// asiVisu includes
#include <asiVisu_Selection.h>

// OCCT includes
#include <BRepAdaptor_Surface.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <math_BullardGenerator.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

// VTK includes
#include <vtkSmartPointer.h>

class asiVisu_PrsManager;

//! Data Model API for geometric part.
class asiEngine_Part : public asiEngine_Base
{
public:

  //! Ctor.
  //! \param[in] model    Data Model instance.
  //! \param[in] prsMgr   Presentation Manager for visual commands.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiEngine_Part(const Handle(asiEngine_Model)&             model,
                 const vtkSmartPointer<asiVisu_PrsManager>& prsMgr,
                 ActAPI_ProgressEntry                       progress = nullptr,
                 ActAPI_PlotterEntry                        plotter  = nullptr)
  //
  : asiEngine_Base(model, progress, plotter), m_prsMgr(prsMgr) {}

  //! Ctor.
  //! \param[in] model    Data Model instance.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiEngine_Part(const Handle(asiEngine_Model)& model,
                 ActAPI_ProgressEntry           progress = nullptr,
                 ActAPI_PlotterEntry            plotter  = nullptr)
  //
  : asiEngine_Base(model, progress, plotter), m_prsMgr(nullptr) {}

public:

  //! \return Part Node.
  asiEngine_EXPORT Handle(asiData_PartNode)
    GetPart();

  //! \return newly created Part Node.
  asiEngine_EXPORT Handle(asiData_PartNode)
    CreatePart();

  //! Imports CAD data from a file with the passed filename into the Part Node.
  //! This function will automatically recognize the file format from its extension
  //! or contents.
  //! \param[in] filename the input filename.
  //! \return true in case of success, false -- otherwise.
  asiEngine_EXPORT bool
    Import(const TCollection_AsciiString& filename);

public:

  asiEngine_EXPORT bool
    CheckDeviation(const Handle(asiData_IVPointSetNode)& pcNode);

  asiEngine_EXPORT bool
    CheckDeviation(const Handle(asiData_IVPointSetNode)& pcNode,
                   Handle(asiData_DeviationNode)&        devNode);

public:

  //! Returns Metadata Node.
  //! \return Metadata Node.
  asiEngine_EXPORT Handle(asiData_MetadataNode)
    GetMetadata() const;

  //! Creates metadata holder.
  //! \return Metadata Node.
  asiEngine_EXPORT Handle(asiData_MetadataNode)
    CreateMetadata();

  //! Cleans up metadata.
  asiEngine_EXPORT void
    CleanMetadata();

  //! Reset part parameters to default.
  asiEngine_EXPORT void
    ResetPartParametersToDefault();

  //! Updates metadata available for the Part Node according to the passed
  //! modification history.
  //! \param[in] history modification history.
  asiEngine_EXPORT void
    UpdateMetadata(const Handle(asiAlgo_History)& history);

  //! \return number of metadata elements.
  asiEngine_EXPORT int
    GetNumOfMetadata() const;

  //! Sets color of the passed subshape.
  asiEngine_EXPORT void
    SetSubshapeColor(const TopoDS_Shape& shape,
                     const int           icolor);

public:

  //! Creates Features Node.
  //! \return Features Node.
  asiEngine_EXPORT Handle(asiData_FeaturesNode)
    CreateFeatures();

  //! Creates Feature Node.
  //! \param[in] name    name of the Node.
  //! \param[in] id      feature ID.
  //! \param[in] feature feature to set.
  //! \return Feature Node.
  asiEngine_EXPORT Handle(asiData_FeatureNode)
    CreateFeature(const TCollection_ExtendedString& name,
                  const int                         id,
                  const TColStd_PackedMapOfInteger& feature);

  //! Cleans up the children of Features Node.
  asiEngine_EXPORT void
    CleanFeatures();

  //! \return number of Feature Nodes.
  asiEngine_EXPORT int
    GetNumOfFeatures() const;

  //! Gathers all Feature Nodes.
  //! \param[out] nodes Feature Nodes.
  asiEngine_EXPORT void
    GetFeatures(Handle(ActAPI_HNodeList)& nodes) const;

  //! Finds or creates Feature Node for the passed ID.
  //! \param[in] featureId feature ID in question.
  //! \param[in] create    whether to create a Node if it does not exist.
  //! \return found or newly created Feature Node.
  asiEngine_EXPORT Handle(asiData_FeatureNode)
    FindFeature(const int  featureId,
                const bool create = false);

public:

  //! Creates Grid 2D Node.
  //! \return Grid 2D Node.
  asiEngine_EXPORT Handle(asiData_Grid2dNode)
    CreateFaceGrid2d();

  //! Finds or creates a Grid 2D Node for the active face.
  //! \param[in] create whether to create a Node if it does not exist.
  //! \return found or newly created Grid 2D Node.
  asiEngine_EXPORT Handle(asiData_Grid2dNode)
    FindFaceGrid2d(const bool create = false);

public:

  //! Creates Discrete Face Node.
  //! \return Discrete Face Node.
  asiEngine_EXPORT Handle(asiData_DiscrFaceNode)
    CreateDiscrFace();

  //! Finds or creates a Discrete Face Node for the active face.
  //! \param[in] create whether to create a Node if it does not exist.
  //! \return found or newly created Discrete Face Node.
  asiEngine_EXPORT Handle(asiData_DiscrFaceNode)
    FindDiscrFace(const bool create = false);

public:

  //! Updates part's geometry in a smart way, so all dependent attributes
  //! are also actualized.
  //! \param[in] model             CAD part to set.
  //! \param[in] history           modification history (if available) to
  //!                              actualize metadata.
  //! \param[in] doResetTessParams indicates whether to reset tessellation
  //!                              parameters.
  //! \return Part Node.
  asiEngine_EXPORT Handle(asiData_PartNode)
    Update(const TopoDS_Shape&            model,
           const Handle(asiAlgo_History)& history = nullptr,
           const bool                     doResetTessParams = false,
           const bool                     doResetUVScaling = true);

  //! Sets the passed face ID to be stored persistently.
  asiEngine_EXPORT void
    SetSelectedFace(const int fid);

  //! Stores the passed AAG in the project tree.
  //! \param[in] aag AAG instance to store.
  asiEngine_EXPORT void
    SetAAG(const Handle(asiAlgo_AAG)& aag);

  asiEngine_EXPORT bool
    HasNaming() const;

  asiEngine_EXPORT void
    InitializeNaming();

  //! Stores history in the Part Node.
  //! \param[in] history history to store.
  asiEngine_EXPORT void
    StoreHistory(const Handle(asiAlgo_History)& history);

  //! Constructs BVH structure for the visualization facets stored in the
  //! part shape.
  //! \param[in] store specifies whether to store BVH in the Node.
  //! \return constructed BVH.
  asiEngine_EXPORT Handle(asiAlgo_BVHFacets)
    BuildBVH(const bool store = true);

  //! Cleans up Data Model structure related to the Part Node.
  asiEngine_EXPORT void
    Clean(const bool cleanMeta = true,
          const bool resetUVScaling = true);

  //! Accessor for a transient pointer to a B-Rep face by its one-based index.
  //! \param[in] oneBasedId one-based index of a face to access.
  //! \return transient pointer to a face.
  asiEngine_EXPORT TopoDS_Face
    GetFace(const int oneBasedId);

  //! Accessor for a transient pointer to a B-Rep edge by its one-based index.
  //! \param[in] oneBasedId one-based index of an edge to access.
  //! \return transient pointer to an edge.
  asiEngine_EXPORT TopoDS_Edge
    GetEdge(const int oneBasedId);

  //! \return shape stored in Part Node.
  asiEngine_EXPORT TopoDS_Shape
    GetShape();

  //! \return AAG stored in the Part Node.
  asiEngine_EXPORT Handle(asiAlgo_AAG)
    GetAAG();

  //! Extracts sub-shape indices for the given collection of face indices.
  //! \param[in]  faceIndices indices of faces.
  //! \param[out] indices     their corresponding indices among all sub-shapes.
  asiEngine_EXPORT void
    GetSubShapeIndicesByFaceIndices(const TColStd_PackedMapOfInteger& faceIndices,
                                    TColStd_PackedMapOfInteger&       indices);

  //! Extracts sub-shape indices for the given collection of edge indices.
  //! \param[in]  edgeIndices indices of edges.
  //! \param[out] indices     their corresponding indices among all sub-shapes.
  asiEngine_EXPORT void
    GetSubShapeIndicesByEdgeIndices(const TColStd_PackedMapOfInteger& edgeIndices,
                                    TColStd_PackedMapOfInteger&       indices);

  //! Extracts sub-shape indices for the given collection of vertex indices.
  //! \param[in]  vertexIndices indices of vertices.
  //! \param[out] indices       their corresponding indices among all sub-shapes.
  asiEngine_EXPORT void
    GetSubShapeIndicesByVertexIndices(const TColStd_PackedMapOfInteger& edgeIndices,
                                      TColStd_PackedMapOfInteger&       indices);

  //! Extracts sub-shape indices for the given collection of sub-shapes.
  //! \param[in]  subShapes sub-shapes of interest.
  //! \param[out] indices   their corresponding IDs.
  asiEngine_EXPORT void
    GetSubShapeIndices(const TopTools_IndexedMapOfShape& subShapes,
                       TColStd_PackedMapOfInteger&       indices);

  //! Extracts sub-shape indices for the given collection of sub-shapes. The
  //! output is distributed by faces, edges and vertices.
  //! \param[in]  subShapes     sub-shapes of interest.
  //! \param[out] faceIndices   global indices for faces.
  //! \param[out] edgeIndices   global indices for edges.
  //! \param[out] vertexIndices global indices for vertices.
  asiEngine_EXPORT void
    GetSubShapeIndices(const TopTools_IndexedMapOfShape& subShapes,
                       TColStd_PackedMapOfInteger&       faceIndices,
                       TColStd_PackedMapOfInteger&       edgeIndices,
                       TColStd_PackedMapOfInteger&       vertexIndices);

  //! Highlights a single face.
  //! \param[in] faceIndex face to highlight.
  asiEngine_EXPORT void
    HighlightFace(const int faceIndex);

  //! Highlights faces.
  //! \param[in] faceIndices faces to highlight.
  asiEngine_EXPORT void
    HighlightFaces(const TColStd_PackedMapOfInteger& faceIndices);

  //! Highlights edges.
  //! \param[in] edgeIndices edges to highlight.
  asiEngine_EXPORT void
    HighlightEdges(const TColStd_PackedMapOfInteger& edgeIndices);

  //! Highlights vertices.
  //! \param[in] vertexIndices vertices to highlight.
  asiEngine_EXPORT void
    HighlightVertices(const TColStd_PackedMapOfInteger& vertexIndices);

  //! Highlights the passed sub-shapes identified by their indices.
  //! \param[in] subShapeIndices indices of the sub-shapes to highlight.
  //! \param[in] selMode         selection mode.
  asiEngine_EXPORT void
    HighlightSubShapes(const TColStd_PackedMapOfInteger& subShapeIndices,
                       const asiVisu_SelectionMode       selMode);

  //! Highlights the passed sub-shapes in Part Viewer.
  //! \param[in] subShapes sub-shapes to highlight.
  asiEngine_EXPORT void
    HighlightSubShapes(const TopTools_IndexedMapOfShape& subShapes);

  //! Retrieves the highlighted sub-shapes from the viewer.
  //! \param[out] subShapes result collection.
  asiEngine_EXPORT void
    GetHighlightedSubShapes(TopTools_IndexedMapOfShape& subShapes);

  //! Retrieves the indices of the highlighted faces in a collection
  //! without repetitions.
  //! \param[out] faceIndices indices of the highlighted faces.
  asiEngine_EXPORT void
    GetHighlightedFaces(TColStd_PackedMapOfInteger& faceIndices);

  //! Retrieves the indices of the highlighted edges in a collection
  //! without repetitions.
  //! \param[out] edgeIndices indices of the highlighted edges.
  asiEngine_EXPORT void
    GetHighlightedEdges(TColStd_PackedMapOfInteger& edgeIndices);

  //! Retrieves the indices of the highlighted vertices in a collection
  //! without repetitions.
  //! \param[out] vertIndices indices of the highlighted vertices.
  asiEngine_EXPORT void
    GetHighlightedVertices(TColStd_PackedMapOfInteger& vertIndices);

  //! Finds a pair face for the given seed face.
  //! \param[in]  aag         the attributed adjacency graph to access faces by IDs.
  //! \param[in]  seedId      the ID of the seed face.
  //! \param[in]  excludedIds the IDs of the faces which are not acceptable as mates.
  //! \param[in]  numSamples  the number of sampled points for checking mates.
  //! \param[in]  inside      indicates whether to cast a ray inside material.
  //! \param[out] mateId      the ID of the found mate face.
  //! \param[out] distance    the deduced distance between seed and mate faces.
  //! \param[out] norm        the deduced norm vector.
  //! \return false if the mate face could not be found.
  template <typename TSurf>
  static bool ComputeMateFace(const Handle(asiAlgo_AAG)& aag,
                              const int                  seedId,
                              const asiAlgo_Feature&     excludedIds,
                              const unsigned             numSamples,
                              const bool                 inside,
                              int&                       mateId,
                              double&                    distance,
                              gp_Vec&                    norm)
  {
    if ( !numSamples )
    {
      return false;
    }

    // Working variables.
    const TopoDS_Face&    seedFace = aag->GetFace(seedId);
    BRepAdaptor_Surface   faceAdt(seedFace);
    const bool            isReversed = (seedFace.Orientation() == TopAbs_REVERSED);
    math_BullardGenerator RNG;

    // Probes:
    // <probe index | face id | distance>
    std::map<int, std::pair<int, gp_Vec>> probes;

    // The nearest face is computed several times.
    for ( unsigned i = 0; i < numSamples; ++i )
    {
      double minDist = Precision::Infinite();

      // Get a random sample point.
      gp_Pnt2d uv;
      if ( !asiAlgo_Utils::GetRandomPoint(seedFace, RNG, uv) )
      {
        return false;
      }

      // Evaluate surface in the sample point to take its norm.
      gp_Pnt P;
      gp_Vec D1U, D1V, N;
      faceAdt.D1(uv.X(), uv.Y(), P, D1U, D1V);

      // Choose norm.
      N = ( inside ? D1V.Crossed(D1U) : D1U.Crossed(D1V) );
      //
      if ( isReversed )
        N.Reverse(); // Reversed face norm.
      //
      if ( N.Magnitude() > gp::Resolution() )
        N.Normalize();

      // Sample ray.
      gp_Lin sampleRay(P, N);

      // Loop over the remaining faces to find a seed's mate.
      Handle(asiAlgo_AAGRandomIterator) aagIt = new asiAlgo_AAGRandomIterator(aag);
      //
      for ( ; aagIt->More() ; aagIt->Next() )
      {
        // Check the next candidate face.
        const int nextId = aagIt->GetFaceId();
        //
        if ( nextId == seedId || excludedIds.Contains(nextId) )
          continue;

        // Skip neighbor faces.
        if ( aag->HasArc( asiAlgo_AAG::t_arc(seedId, nextId) ) )
          continue;

        // Skip faces of improper type.
        const TopoDS_Face& nextFace = aag->GetFace(nextId);
        //
        if ( !asiAlgo_Utils::IsTypeOf<TSurf>(nextFace) )
          continue;

        // Check that the casted ray intersects the face's AABB. This check
        // allows early filtering out geometrically unpromising candidates.
        Bnd_Box nextFaceAABB;
        asiAlgo_Utils::Bounds(nextFace, true, true, nextFaceAABB);
        //
        if ( nextFaceAABB.IsOut(sampleRay) )
          continue; // Exclude face from consideration.

        // Find precise intersection.
        IntCurvesFace_ShapeIntersector interRayFace;
        interRayFace.Load(nextFace, 1.0e-4);
        interRayFace.Perform(sampleRay, 0.0, 1e100);
        //
        if ( !interRayFace.IsDone() )
          continue;
        //
        if ( interRayFace.NbPnt() == 0 )
          continue;

        for ( int solId = 1; solId <= interRayFace.NbPnt(); ++solId )
        {
          const double nextDist = interRayFace.Pnt(solId).Distance(P);
          //
          if ( nextDist < minDist )
          {
            probes[i] = std::pair<int, gp_Vec>(nextId, gp_Vec(N)*nextDist);
            minDist   = nextDist;
          }
        }
      }
    }

    /* ==============================================
     *  Votes: take a face that's the most voted for.
     * ============================================== */

    // Count votes:
    // <face id | occurrences>
    std::map<int, int> dup;
    //
    for ( auto probe : probes )
    {
      const int fid = probe.second.first;
      dup[fid]++;
    }

    // Select the winner face id.
    int maxVotes = 0;
    int maxVotedFid = 0;
    for ( auto tuple : dup )
    {
      if ( tuple.second > maxVotes )
      {
        maxVotedFid = tuple.first;
        maxVotes    = tuple.second;
      }
    }

    // Initialize the outputs: mate id, distance.
    mateId = maxVotedFid;
    for ( auto probe : probes )
    {
      const int fid = probe.second.first;
      if ( fid == maxVotedFid )
      {
        distance = probe.second.second.Magnitude();

        if ( Abs(distance) > Precision::Confusion() )
          norm = probe.second.second.Normalized();

        break; // Let's take just the first distance for now.
      }
    }

    return mateId > 0;
  }

  //! Passes metadata from the Part Node to the given XDE document.
  //! This method is to be used for XDE-based data tranlators.
  //! \param[in] pid    the target part in the XDE document.
  //! \param[in] xdeDoc the target XDE document to set metadata to.
  asiEngine_EXPORT void
    TransferMetadata(const asiAsm::xde::PartId&      pid,
                     const Handle(asiAsm::xde::Doc)& xdeDoc);

protected:

  vtkSmartPointer<asiVisu_PrsManager> m_prsMgr; //!< Presentation Manager.

};

#endif
