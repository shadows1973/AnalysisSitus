//-----------------------------------------------------------------------------
// Created on: 07 November 2016 (99 years of October Revolution)
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

// Own include
#include <asiUI_ViewerPartListener.h>

// asiUI includes
#include <asiUI_BgColorDialog.h>
#include <asiUI_Common.h>
#include <asiUI_DialogDump.h>

// asiAlgo includes
#include <asiAlgo_CheckDihedralAngle.h>
#include <asiAlgo_InvertFaces.h>
#include <asiAlgo_JSON.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_ShapeSerializer.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>

// asiEngine includes
#include <asiEngine_Features.h>
#include <asiEngine_IV.h>
#include <asiEngine_Part.h>
#include <asiEngine_Triangulation.h>

// asiVisu includes
#include <asiVisu_PartNodeInfo.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

// VTK includes
#pragma warning(push, 0)
#include <vtkCamera.h>
#pragma warning(pop)

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QClipboard>
#pragma warning(pop)

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

namespace
{
#if defined USE_MOBIUS

  Handle(Poly_Triangulation)
    ExtractRegion(const t_ptr<poly_Mesh>&           tris,
                  const TColStd_PackedMapOfInteger& ids)
  {
    t_ptr<poly_Mesh> region = new poly_Mesh;

    // Add new nodes.
    std::unordered_map<poly_VertexHandle, poly_VertexHandle> nodeMap;
    //
    for ( TColStd_PackedMapOfInteger::Iterator fit(ids); fit.More(); fit.Next() )
    {
      const int tid = fit.Key();

      poly_Triangle t;
      tris->GetTriangle(poly_TriangleHandle(tid), t);

      poly_VertexHandle globalNodeHandles[3];
      t.GetVertices(globalNodeHandles[0], globalNodeHandles[1], globalNodeHandles[2]);

      for ( int k = 0; k < 3; ++k )
      {
        if ( nodeMap.find(globalNodeHandles[k]) == nodeMap.end() )
        {
          t_xyz V;
          tris->GetVertex(globalNodeHandles[k], V);

          poly_VertexHandle localNodeHandle = region->AddVertex(V);

          nodeMap.insert({globalNodeHandles[k], localNodeHandle});
        }
      }
    }

    // Add new triangles.
    std::vector<poly_Triangle> newTriangles;
    //
    for ( TColStd_PackedMapOfInteger::Iterator fit(ids); fit.More(); fit.Next() )
    {
      const int tid = fit.Key();

      poly_Triangle t;
      tris->GetTriangle(poly_TriangleHandle(tid), t);

      poly_VertexHandle globalNodeHandles[3];
      t.GetVertices(globalNodeHandles[0], globalNodeHandles[1], globalNodeHandles[2]);

      region->AddTriangle( nodeMap[globalNodeHandles[0]],
                           nodeMap[globalNodeHandles[1]],
                           nodeMap[globalNodeHandles[2]] );
    }

    // Set result and return.
    return cascade::GetOpenCascadeMesh(region);
  }

#endif

  //! Prepares one shape out of the passed collection of faces. Is there
  //! is only one face passed, it will be returned without any processing.
  //! For multiple faces, a compound is constructed and returned.
  //! \param[in] faces the faces to process.
  //! \return one shape.
  TopoDS_Shape FacesAsOneShape(const TopTools_IndexedMapOfShape& faces)
  {
    TopoDS_Shape oneShape;
    //
    if ( faces.Extent() == 1 )
    {
      oneShape = faces(1);
    }
    else
    {
      // Put faces in a compound.
      TopoDS_Compound comp;
      BRep_Builder().MakeCompound(comp);
      //
      for ( int k = 1; k <= faces.Extent(); ++k )
        BRep_Builder().Add( comp, faces(k) );
      //
      oneShape = comp;
    }

    return oneShape;
  }

  //! Prepares one mesh out of the passed collection of faces.
  //! \param[in] faces the faces to process.
  //! \return one bulk of mesh.
  Handle(Poly_Triangulation) FacesAsOneMesh(const TopTools_IndexedMapOfShape& faces)
  {
    asiAlgo_MeshMerge::t_faceElems history;
    return asiAlgo_MeshMerge::PutTogether( FacesAsOneShape(faces), history );
  }
}

//-----------------------------------------------------------------------------

asiUI_ViewerPartListener::asiUI_ViewerPartListener(asiUI_ViewerPart*              wViewerPart,
                                                   asiUI_ViewerDomain*            wViewerDomain,
                                                   asiUI_ViewerHost*              wViewerHost,
                                                   asiUI_ObjectBrowser*           wBrowser,
                                                   const Handle(asiEngine_Model)& model,
                                                   ActAPI_ProgressEntry           progress,
                                                   ActAPI_PlotterEntry            plotter)
//
: asiUI_Viewer3dListener  (wViewerPart, model, progress, plotter),
  m_wViewerDomain         (wViewerDomain),
  m_wViewerHost           (wViewerHost),
  m_wBrowser              (wBrowser),
  m_pSaveBREPAction       (nullptr),
  m_pSaveSTLAction        (nullptr),
  m_pShowNormsAction      (nullptr),
  m_pInvertFacesAction    (nullptr),
  m_pShowOriContourAction (nullptr),
  m_pShowHatchingAction   (nullptr),
  m_pCopyAsStringAction   (nullptr),
  m_pSetAsVariableAction  (nullptr),
  m_pFindIsolated         (nullptr),
  m_pCheckDihAngle        (nullptr),
  m_pAddAsFeature         (nullptr),
  m_pGetAsBLOB            (nullptr),
  m_pMeasureLength        (nullptr)
{}

//-----------------------------------------------------------------------------

asiUI_ViewerPartListener::~asiUI_ViewerPartListener()
{}

//-----------------------------------------------------------------------------

void asiUI_ViewerPartListener::Connect()
{
  asiUI_Viewer3dListener::Connect(); // Connect basic reactions.

  connect( m_pViewer, SIGNAL ( facePicked(asiVisu_PickerResult*) ),
           this,      SLOT   ( onFacePicked(asiVisu_PickerResult*) ) );
  //
  connect( m_pViewer, SIGNAL ( edgePicked(asiVisu_PickerResult*) ),
           this,      SLOT   ( onEdgePicked(asiVisu_PickerResult*) ) );
  //
  connect( m_pViewer, SIGNAL ( vertexPicked(asiVisu_PickerResult*) ),
           this,      SLOT   ( onVertexPicked(asiVisu_PickerResult*) ) );
}

//-----------------------------------------------------------------------------

void asiUI_ViewerPartListener::onFacePicked(asiVisu_PickerResult* pickRes)
{
  // Check if part is picked
  asiVisu_PartNodeInfo* nodeInfo = asiVisu_PartNodeInfo::Retrieve( pickRes->GetPickedActor() );
  //
  if ( pickRes->GetPickedActor() && !nodeInfo )
    return;

  Handle(asiData_PartNode) geom_n = m_model->GetPartNode();
  //
  if ( m_wViewerDomain )
    m_wViewerDomain->PrsMgr()->Actualize(geom_n->GetFaceRepresentation().get(), false, true);
  //
  if ( m_wViewerHost )
    m_wViewerHost->PrsMgr()->Actualize(geom_n->GetSurfaceRepresentation().get(), false, true);

  /* =============================
   *  Dump textual info to logger
   * ============================= */

  // Get index of the active sub-shape.
  Handle(TColStd_HPackedMapOfInteger)
    gids = geom_n->GetFaceRepresentation()->GetSelectedFaces();
  //
  if ( gids.IsNull() || gids->Map().IsEmpty() )
    return;

  // Get sub-shapes map.
  const TopTools_IndexedMapOfShape&
    allSubShapes = geom_n->GetAAG()->RequestMapOfSubShapes();

  // Get map of faces.
  const TopTools_IndexedMapOfShape&
    allFaces = geom_n->GetAAG()->GetMapOfFaces();

  // Loop over the selected faces.
  TColStd_PackedMapOfInteger fids;
  //
  for ( TColStd_PackedMapOfInteger::Iterator git( gids->Map() ); git.More(); git.Next() )
  {
    const int globalId = git.Key();
    //
    if ( globalId < 1 || globalId > allSubShapes.Extent() )
      continue;

    // Get sub-shape.
    const TopoDS_Shape& subShape = allSubShapes(globalId);

    // Get pedigree index.
    const int pedigreeId = allFaces.FindIndex(subShape);
    fids.Add(pedigreeId);

    // Send message to logger.
    TCollection_AsciiString
      msg = asiAlgo_Utils::NamedShapeToString( subShape,
                                               pedigreeId,
                                               globalId,
                                               geom_n->GetNaming() );
    //
    m_progress.SendLogMessage( LogInfo(Normal) << msg.ToCString() );
  }

  if ( fids.Extent() > 1 )
    m_progress.SendLogMessage( LogInfo(Normal) << "Selected faces: %1." << fids );
}

//-----------------------------------------------------------------------------

void asiUI_ViewerPartListener::onEdgePicked(asiVisu_PickerResult* pickRes)
{
  // Check if part is picked
  asiVisu_PartNodeInfo* nodeInfo = asiVisu_PartNodeInfo::Retrieve( pickRes->GetPickedActor() );
  //
  if ( pickRes->GetPickedActor() && !nodeInfo )
    return;

  Handle(asiData_PartNode) geom_n = m_model->GetPartNode();
  //
  if ( m_wViewerDomain )
    m_wViewerDomain->PrsMgr()->Actualize(geom_n->GetEdgeRepresentation().get(), false, true);
  //
  if ( m_wViewerHost )
    m_wViewerHost->PrsMgr()->Actualize(geom_n->GetCurveRepresentation().get(), false, true);

  /* =============================
   *  Dump textual info to logger
   * ============================= */

  // Get index of the active sub-shape.
  const int
    globalId = geom_n->GetEdgeRepresentation()->GetSelectedEdge();
  //
  if ( globalId == 0 )
    return;

  // Get sub-shapes map.
  const TopTools_IndexedMapOfShape&
    allSubShapes = geom_n->GetAAG()->RequestMapOfSubShapes();
  //
  if ( globalId < 1 || globalId > allSubShapes.Extent() )
    return;

  // Get sub-shape.
  const TopoDS_Shape& subShape = allSubShapes(globalId);

  // Get map of edges.
  const TopTools_IndexedMapOfShape&
    allEdges = geom_n->GetAAG()->RequestMapOfEdges();
  //
  const int pedigreeId = allEdges.FindIndex(subShape);

  // Send message to logger.
  TCollection_AsciiString
    msg = asiAlgo_Utils::NamedShapeToString( subShape,
                                             pedigreeId,
                                             globalId,
                                             geom_n->GetNaming() );
  //
  m_progress.SendLogMessage( LogInfo(Normal) << msg.ToCString() );
}

//-----------------------------------------------------------------------------

void asiUI_ViewerPartListener::onVertexPicked(asiVisu_PickerResult* pickRes)
{
  // Check if part is picked
  asiVisu_PartNodeInfo* nodeInfo = asiVisu_PartNodeInfo::Retrieve( pickRes->GetPickedActor() );
  //
  if ( pickRes->GetPickedActor() && !nodeInfo )
    return;

  Handle(asiData_PartNode) geom_n = m_model->GetPartNode();

  /* =============================
   *  Dump textual info to logger
   * ============================= */

  // Get index of the active sub-shape.
  const int
    globalId = geom_n->GetVertexRepresentation()->GetSelectedVertex();
  //
  if ( globalId == 0 )
    return;

  // Get sub-shapes map.
  const TopTools_IndexedMapOfShape&
    allSubShapes = geom_n->GetAAG()->RequestMapOfSubShapes();
  //
  if ( globalId < 1 || globalId > allSubShapes.Extent() )
    return;

  // Get sub-shape.
  const TopoDS_Shape& subShape = allSubShapes(globalId);

  // Get map of vertices.
  const TopTools_IndexedMapOfShape&
    allVertices = geom_n->GetAAG()->RequestMapOfVertices();
  //
  const int pedigreeId = allVertices.FindIndex(subShape);

  // Send message to logger.
  TCollection_AsciiString
    msg = asiAlgo_Utils::NamedShapeToString( subShape,
                                             pedigreeId,
                                             globalId,
                                             geom_n->GetNaming() );
  //
  m_progress.SendLogMessage( LogInfo(Normal) << msg.ToCString() );
}

//-----------------------------------------------------------------------------

void asiUI_ViewerPartListener::populateMenu(QMenu& menu)
{
  asiEngine_Part          partApi ( m_model, m_pViewer->PrsMgr() );
  asiEngine_Triangulation trisApi ( m_model, m_pViewer->PrsMgr() );

  // Get highlighted faces and edges.
  TColStd_PackedMapOfInteger faceIndices, edgeIndices, vertIndices, facetIndices;
  //
  partApi.GetHighlightedFaces    (faceIndices);
  partApi.GetHighlightedEdges    (edgeIndices);
  partApi.GetHighlightedVertices (vertIndices);
  trisApi.GetHighlightedFacets   (facetIndices);

  // Get Part Node.
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "Part Node is null or bad-formed." );
    return;
  }

  // Prepare the context menu items.
  if ( facetIndices.Extent() )
  {
    menu.addSeparator();
    m_pSaveSTLAction = menu.addAction("Save to STL...");
  }

  // Prepare the context menu items.
  if ( faceIndices.Extent() || edgeIndices.Extent() || vertIndices.Extent() )
  {
    // Add items specific to faces.
    if ( faceIndices.Extent() )
    {
      menu.addSeparator();
      //
      if ( m_pViewer->PrsMgr()->IsPresentable( STANDARD_TYPE(asiData_FaceNormsNode) ) )
      {
        m_pShowNormsAction = menu.addAction("Show face normals");
      }
      if ( m_pViewer->PrsMgr()->IsPresentable( STANDARD_TYPE(asiData_FaceContourNode) ) )
      {
        m_pShowOriContourAction = menu.addAction("Show face oriented contour");
      }
      if ( m_pViewer->PrsMgr()->IsPresentable( STANDARD_TYPE(asiData_HatchingNode) ) )
      {
        m_pShowHatchingAction = menu.addAction("Show hatching");
      }
      //
      m_pInvertFacesAction = menu.addAction("Invert faces");
      m_pFindIsolated      = menu.addAction("Find isolated");
      m_pCheckDihAngle     = menu.addAction("Check dihedral angle");
      m_pAddAsFeature      = menu.addAction("Add as feature");
      m_pGetAsBLOB         = menu.addAction("Get as BLOB");
    }

    menu.addSeparator();
    //
    m_pSaveBREPAction      = menu.addAction("Save to BREP...");
    m_pSetAsVariableAction = menu.addAction("Set as variable");

    if ( faceIndices.Extent() )
      m_pSaveSTLAction = menu.addAction("Save to STL...");

    // Add items which work for single-element selection.
    if ( faceIndices.Extent() == 1 || edgeIndices.Extent() == 1 )
    {
      m_pCopyAsStringAction = menu.addAction("Copy as JSON");
    }

    // Selected items are vertices.
    if ( (vertIndices.Extent() == 2) ||
         (edgeIndices.Extent() == 2) ||
         (faceIndices.Extent() == 2) )
    {
      m_pMeasureLength = menu.addAction("Measure distance");
    }
  }
}

//-----------------------------------------------------------------------------

void asiUI_ViewerPartListener::executeAction(QAction* pAction)
{
  if ( !pAction )
    return;

  //---------------------------------------------------------------------------
  // ACTION: save BREP
  //---------------------------------------------------------------------------
  if ( pAction == m_pSaveBREPAction )
  {
    // Get highlighted sub-shapes
    TopTools_IndexedMapOfShape selected;
    asiEngine_Part( m_model, m_pViewer->PrsMgr() ).GetHighlightedSubShapes(selected);

    // Let user choose a filename
    QString filename = asiUI_Common::selectBRepFile(asiUI_Common::OpenSaveAction_Save);
    //
    if ( filename.isEmpty() )
      return;

    // Prepare a shape to dump
    TopoDS_Shape shape2Save = ::FacesAsOneShape(selected);

    // Save shape
    if ( !asiAlgo_Utils::WriteBRep( shape2Save, QStr2AsciiStr(filename) ) )
    {
      m_progress.SendLogMessage( LogErr(Normal) << "Cannot save shape." );
      return;
    }
  }

  //---------------------------------------------------------------------------
  // ACTION: save STL
  //---------------------------------------------------------------------------
  if ( pAction == m_pSaveSTLAction )
  {
    asiEngine_Part          partApi( m_model, m_pViewer->PrsMgr() );
    asiEngine_Triangulation trisApi( m_model, m_pViewer->PrsMgr() );

    // Get highlighted elements
    TopTools_IndexedMapOfShape selectedShapes;
    TColStd_PackedMapOfInteger selectedFacets;
    //
    partApi.GetHighlightedSubShapes(selectedShapes);
    trisApi.GetHighlightedFacets(selectedFacets);

    // Let user choose a filename
    QString filename = asiUI_Common::selectSTLFile(asiUI_Common::OpenSaveAction_Save);
    //
    if ( filename.isEmpty() )
      return;

    // Prepare a triangulation to dump
    Handle(Poly_Triangulation) mesh2Save;
    //
    if ( selectedShapes.Extent() )
    {
      mesh2Save = ::FacesAsOneMesh(selectedShapes);
    }
#if defined USE_MOBIUS
    else
    {
      mesh2Save = ::ExtractRegion( trisApi.GetTriangulation(),
                                   selectedFacets );
    }
#endif

    // Save mesh
    if ( !asiAlgo_Utils::WriteStl( mesh2Save, QStr2AsciiStr(filename) ) )
    {
      m_progress.SendLogMessage( LogErr(Normal) << "Cannot save triangulation." );
      return;
    }
  }

  //---------------------------------------------------------------------------
  // ACTION: copy as string
  //---------------------------------------------------------------------------
  else if ( pAction == m_pCopyAsStringAction )
  {
    // Get highlighted sub-shapes.
    TopTools_IndexedMapOfShape selected;
    asiEngine_Part( m_model, m_pViewer->PrsMgr() ).GetHighlightedSubShapes(selected);

    // Dump to JSON string.
    TCollection_AsciiString jsonStr;
    //
    for ( int k = 1; k <= selected.Extent(); ++k )
    {
      std::ostringstream jsonStream;

      // Get selected shape.
      const TopoDS_Shape& selectedSh = selected(k);

      if ( selectedSh.ShapeType() == TopAbs_EDGE )
      {
        const TopoDS_Edge& selectedEdge = TopoDS::Edge(selectedSh);

        // Get curve.
        double f, l;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(selectedEdge, f, l);

        // Dump.
        asiAlgo_JSON::DumpCurve(curve, jsonStream);
      }
      else if ( selectedSh.ShapeType() == TopAbs_FACE )
      {
        const TopoDS_Face& selectedFace = TopoDS::Face(selectedSh);

        // Get surface.
        Handle(Geom_Surface) surface = BRep_Tool::Surface(selectedFace);

        // Dump.
        asiAlgo_JSON::DumpSurface(surface, jsonStream);
      }

      jsonStr += jsonStream.str().c_str();
    }

    // Set to clipboard.
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText( AsciiStr2QStr(jsonStr) );

    // Notify.
    m_progress.SendLogMessage( LogInfo(Normal) << "JSON was copied to clipboard." );
  }

  //---------------------------------------------------------------------------
  // ACTION: show normal field
  //---------------------------------------------------------------------------
  else if ( pAction == m_pShowNormsAction )
  {
    TIMER_NEW
    TIMER_GO

    m_pViewer->PrsMgr()->Actualize( m_model->GetPartNode()->GetNormsRepresentation() );

    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Visualization of normals")
  }

  //---------------------------------------------------------------------------
  // ACTION: show oriented contour
  //---------------------------------------------------------------------------
  else if ( pAction == m_pShowOriContourAction )
  {
    TIMER_NEW
    TIMER_GO

    m_pViewer->PrsMgr()->Actualize( m_model->GetPartNode()->GetContourRepresentation() );

    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Visualization of oriented contour")
  }

  //---------------------------------------------------------------------------
  // ACTION: show face hatching
  //---------------------------------------------------------------------------
  else if ( pAction == m_pShowHatchingAction )
  {
    TIMER_NEW
    TIMER_GO

    m_pViewer->PrsMgr()->Actualize( m_model->GetPartNode()->GetHatchingRepresentation() );

    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Visualization of face hatching")
  }

  //---------------------------------------------------------------------------
  // ACTION: invert faces
  //---------------------------------------------------------------------------
  else if ( pAction == m_pInvertFacesAction )
  {
    // Get highlighted faces
    TColStd_PackedMapOfInteger faceIndices;
    asiEngine_Part( m_model, m_pViewer->PrsMgr() ).GetHighlightedFaces(faceIndices);

    // Get Part Node
    Handle(asiData_PartNode) part_n = m_model->GetPartNode();
    //
    if ( part_n.IsNull() || !part_n->IsWellFormed() )
    {
      m_progress.SendLogMessage( LogErr(Normal) << "Part Node is null or bad-formed" );
      return;
    }

    TIMER_NEW
    TIMER_GO

    asiAlgo_InvertFaces InvertFaces( part_n->GetAAG() );
    //
    if ( !InvertFaces.Perform(faceIndices) )
    {
      m_progress.SendLogMessage( LogErr(Normal) << "Face inversion failed" );
      return;
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Invert faces")

    // Update Data Model
    m_model->OpenCommand();
    {
      asiEngine_Part(m_model).Update( InvertFaces.GetResult() );
    }
    m_model->CommitCommand();

    // Actualize
    m_pViewer->PrsMgr()->Actualize(part_n);
    m_pViewer->PrsMgr()->Actualize( m_model->GetPartNode()->GetNormsRepresentation() );
  }

  //---------------------------------------------------------------------------
  // ACTION: set as variable
  //---------------------------------------------------------------------------
  if ( pAction == m_pSetAsVariableAction )
  {
    // Get highlighted sub-shapes
    TopTools_IndexedMapOfShape selected;
    asiEngine_Part( m_model, m_pViewer->PrsMgr() ).GetHighlightedSubShapes(selected);

    // Prepare a shape to set as a variable
    TopoDS_Shape shape2Var = ::FacesAsOneShape(selected);

    // Add variable via the imperative plotter
    m_plotter.DRAW_SHAPE(shape2Var, Color_Yellow, "var");
  }

  //---------------------------------------------------------------------------
  // ACTION: find isolated
  //---------------------------------------------------------------------------
  else if ( pAction == m_pFindIsolated )
  {
    asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

    // Get highlighted faces
    asiAlgo_Feature faceIndices;
    partApi.GetHighlightedFaces(faceIndices);

    // Find features.
    asiAlgo_Feature
      isolated = asiEngine_Features(m_model,
                                    m_progress,
                                    m_plotter).FindIsolated(faceIndices);

    if ( !isolated.IsEmpty() )
    {
      partApi.HighlightFaces(isolated);
      //
      m_progress.SendLogMessage(LogInfo(Normal) << "Isolated faces: %1."
                                                << isolated);
    }
    else
      m_progress.SendLogMessage(LogInfo(Normal) << "No isolated features found.");
  }

  //---------------------------------------------------------------------------
  // ACTION: check dihedral angle
  //---------------------------------------------------------------------------
  else if ( pAction == m_pCheckDihAngle )
  {
    asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

    // Get highlighted faces.
    asiAlgo_Feature faceIndices;
    partApi.GetHighlightedFaces(faceIndices);

    if ( faceIndices.Extent() != 2 )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "There should be two faces to measure an angle.");
      return;
    }

    // Get AAG to access faces by indices.
    Handle(asiAlgo_AAG) aag = partApi.GetAAG();
    //
    if ( aag.IsNull() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "AAG is null.");
      return;
    }

    // Get faces.
    const TopoDS_Face& F = aag->GetFace( faceIndices.GetMinimalMapped() );
    const TopoDS_Face& G = aag->GetFace( faceIndices.GetMaximalMapped() );

    double xMin[2], yMin[2], zMin[2], xMax[2], yMax[2], zMax[2];
    asiAlgo_Utils::Bounds(F, xMin[0], yMin[0], zMin[0], xMax[0], yMax[0], zMax[0]);
    asiAlgo_Utils::Bounds(G, xMin[1], yMin[1], zMin[1], xMax[1], yMax[1], zMax[1]);
    //
    const double
      faceSize[2] = { gp_Pnt(xMin[0], yMin[0], zMin[0]).Distance( gp_Pnt(xMax[0], yMax[0], zMax[0]) ),
                      gp_Pnt(xMin[1], yMin[1], zMin[1]).Distance( gp_Pnt(xMax[1], yMax[1], zMax[1]) ) };
    //
    const double
      glyphCoeff = Max(faceSize[0], faceSize[1])*0.1;

    // Measure the angle.
    double                     angleRad = 0.;
    TopTools_IndexedMapOfShape commonEdges;
    gp_Pnt                     FP, GP;
    gp_Vec                     FN, GN;
    //
    asiAlgo_CheckDihedralAngle angChecker;
    //
    asiAlgo_FeatureAngleType
      angleType = angChecker.AngleBetweenFaces(F, G, false, 1.0e-3,
                                               commonEdges, angleRad,
                                               FP, GP, FN, GN);

    if ( !commonEdges.Extent() )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "The selected faces are not adjacent.");
      return;
    }

    m_progress.SendLogMessage( LogInfo(Normal) << "Angle is %1 degrees, %2."
                                               << Abs(angleRad)*180./M_PI
                                               << asiAlgo_Utils::FeatureAngleToString(angleType) );

    double colorR, colorG, colorB;
    asiVisu_Utils::ColorForFeatureAngle(angleType, colorR, colorG, colorB);
    //
    for ( int eidx = 1; eidx <= commonEdges.Extent(); ++eidx )
    {
      m_plotter.REDRAW_SHAPE("vexity",
                             commonEdges(eidx),
                             ActAPI_Color(colorR,
                                          colorG,
                                          colorB,
                                          Quantity_TOC_RGB),
                             1.0,
                             true);
    }
    //
    m_plotter.REDRAW_VECTOR_AT("FN", FP, FN*glyphCoeff, Color_Red);
    m_plotter.REDRAW_VECTOR_AT("GN", GP, GN*glyphCoeff, Color_Red);
  }

  //---------------------------------------------------------------------------
  // ACTION: add as feature
  //---------------------------------------------------------------------------
  else if ( pAction == m_pAddAsFeature )
  {
    asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

    // Get highlighted faces.
    asiAlgo_Feature faceIndices;
    partApi.GetHighlightedFaces(faceIndices);

    // Add Data Node.
    m_model->OpenCommand();
    {
      const int numFeatures = partApi.GetNumOfFeatures();

      // Get feature to store the recognition result.
      Handle(asiData_FeatureNode)
        featureNode = partApi.FindFeature(numFeatures + 1, true);

      // Store indices.
      featureNode->SetMask(faceIndices);
    }
    m_model->CommitCommand();

    // Update object browser.
    if ( m_wBrowser )
      m_wBrowser->Populate();
  }

  //---------------------------------------------------------------------------
  // ACTION: get as BLOB
  //---------------------------------------------------------------------------
  else if ( pAction == m_pGetAsBLOB )
  {
    asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

    // Get highlighted sub-shapes.
    TopTools_IndexedMapOfShape selected;
    asiEngine_Part( m_model, m_pViewer->PrsMgr() ).GetHighlightedSubShapes(selected);

    // Get shape to serialize.
    TopoDS_Shape shape2Serialize = ::FacesAsOneShape(selected);

    // Serialize and dump.
    std::string buff;
    if ( asiAlgo_ShapeSerializer::Serialize(shape2Serialize, buff) )
    {
      asiUI_DialogDump* pDumpDlg = new asiUI_DialogDump("Shape BLOB (base64-encoded)", m_progress);
      pDumpDlg->Populate(buff);
      pDumpDlg->show();
    }
    else
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Cannot dump the shape.");
    }
  }

  //---------------------------------------------------------------------------
  // ACTION: measure distance
  //---------------------------------------------------------------------------
  else if ( pAction == m_pMeasureLength )
  {
    asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

    // Get highlighted subshapes.
    TopTools_IndexedMapOfShape ssIndices;
    partApi.GetHighlightedSubShapes(ssIndices);

    // Distance between two subshapes.
    if ( ssIndices.Extent() == 2 )
    {
      const TopoDS_Shape& S1 = ssIndices(1);
      const TopoDS_Shape& S2 = ssIndices(2);

      BRepExtrema_DistShapeShape extSS(S1, S2);
      //
      if ( !extSS.IsDone() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Distance computation is not done.");
        return;
      }

      for ( int isol = 1; isol <= extSS.NbSolution(); ++isol )
      {
        gp_Pnt P1 = extSS.PointOnShape1(isol);
        gp_Pnt P2 = extSS.PointOnShape2(isol);

        gp_Trsf T = partApi.GetPart()->GetTransformationMx();

        P1.Transform(T);
        P2.Transform(T);

        TCollection_AsciiString distName("distance");
        distName += isol;

        m_plotter.REDRAW_LINK(distName, P1, P2, Color_White);
        m_progress.SendLogMessage( LogInfo(Normal) << "Distance between shapes: %1."
                                                   << P1.Distance(P2) );
      }
    }
  }
}
