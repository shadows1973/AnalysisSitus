//-----------------------------------------------------------------------------
// Created on: 05 December 2015
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <exeFeatures_TopoGraph.h>

// exeFeatures includes
#include <exeFeatures_AAGAdaptor.h>
#include <exeFeatures_CommonFacilities.h>
#include <exeFeatures_TopoGraphItem.h>

// asiVisu includes
#include <asiVisu_Utils.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// asiEngine includes
#include <asiEngine_Part.h>

// OCCT includes
#include <TopoDS_Iterator.hxx>

// VTK includes
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkContextActor.h>
#include <vtkContextInteractorStyle.h>
#include <vtkContextScene.h>
#include <vtkContextTransform.h>
#include <vtkGraphLayout.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSimple2DLayoutStrategy.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>

#define LEGEND_TITLE_ACCESSORY "Topology graph"
#define LEGEND_TITLE_ADJACENCY "AAG"

#define VTK_CREATE(Type, Name) \
  vtkSmartPointer<Type> Name = vtkSmartPointer<Type>::New()

//! Constructor.
exeFeatures_TopoGraph::exeFeatures_TopoGraph()
: m_textWidget    (NULL),
  m_summaryWidget (NULL)
{}

//! Destructor.
exeFeatures_TopoGraph::~exeFeatures_TopoGraph()
{
}

//! Renders graph.
//! \param graph  [in] VTK presentable graph.
//! \param shape  [in] master shape.
//! \param regime [in] kind of graph to render.
void exeFeatures_TopoGraph::Render(const vtkSmartPointer<vtkGraph>& graph,
                                   const TopoDS_Shape&              shape,
                                   const Regime                     regime)
{
  /* ===================================
   *  Prepare structures for attributes
   * =================================== */

  // Layout strategy
  vtkNew<vtkSimple2DLayoutStrategy> simple2DStrategy;
  simple2DStrategy->SetIterationsPerLayout(10);

  // Layout
  vtkSmartPointer<vtkGraphLayout> graphLayout = vtkSmartPointer<vtkGraphLayout>::New();
  graphLayout->SetInputData(graph);
  graphLayout->SetLayoutStrategy( simple2DStrategy.GetPointer() );
  graphLayout->Update();

  // Graph item
  vtkSmartPointer<exeFeatures_TopoGraphItem> graphItem = vtkSmartPointer<exeFeatures_TopoGraphItem>::New();
  graphItem->SetGraph( graphLayout->GetOutput() );

  connect( graphItem, SIGNAL( vertexPicked(const int, const vtkIdType) ),
           this,      SLOT( onVertexPicked(const int, const vtkIdType) ) );

  // Context transform
  vtkSmartPointer<vtkContextTransform> trans = vtkSmartPointer<vtkContextTransform>::New();
  trans->SetInteractive(true);
  trans->AddItem(graphItem);

  // Context actor
  vtkSmartPointer<vtkContextActor> actor = vtkSmartPointer<vtkContextActor>::New();
  actor->GetScene()->AddItem(trans);

  /* ===============================================
   *  Prepare and initialize interaction facilities
   * =============================================== */

  // Renderer
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->SetGradientBackground(true);
  renderer->SetBackground(0.0, 0.0, 0.0);
  renderer->SetBackground(0.1, 0.1, 0.1);

  // Render window
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderer->AddActor(actor);

  // Context interactor style
  vtkNew<vtkContextInteractorStyle> interactorStyle;
  interactorStyle->SetScene( actor->GetScene() );

  // Interactor
  vtkNew<QVTKInteractor> interactor;
  interactor->SetInteractorStyle( interactorStyle.GetPointer() );
  interactor->SetRenderWindow( renderWindow.GetPointer() );

  // Create Qt window. NOTICE that interactor should be already given to
  // the render window at this stage
  m_pWidget = new asiUI_VtkWindow();
  m_pWidget->SetRenderWindow( renderWindow.GetPointer() );

  connect( m_pWidget, SIGNAL( windowClosed() ), this, SLOT( onViewerClosed() ) );

  /* ========================
   *  Add legend and summary
   * ======================== */

  // Legend
  m_textWidget = vtkTextWidget::New();
  vtkTextRepresentation* textRep = vtkTextRepresentation::SafeDownCast( m_textWidget->GetRepresentation() );
  textRep->GetPositionCoordinate()->SetValue(0.2, 0.01);
  textRep->GetPosition2Coordinate()->SetValue(0.5, 0.06);
  m_textWidget->SelectableOff();
  //
  vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
  textRep->SetTextActor(textActor);
  //
  m_textWidget->GetTextActor()->SetInput( regime == Regime_Full ? LEGEND_TITLE_ACCESSORY : LEGEND_TITLE_ADJACENCY );
  m_textWidget->SetInteractor( m_pWidget->GetInteractor() );
  m_textWidget->SetDefaultRenderer( renderer );
  m_textWidget->SetCurrentRenderer( renderer );

  // Shape summary
  m_summaryWidget = vtkTextWidget::New();
  asiVisu_Utils::InitTextWidget(m_summaryWidget);
  //
  TCollection_AsciiString shapeInfo;
  asiAlgo_Utils::ShapeSummary(shape, shapeInfo);
  m_summaryWidget->GetTextActor()->SetInput( shapeInfo.ToCString() );
  //
  m_summaryWidget->SetInteractor      ( m_pWidget->GetInteractor() );
  m_summaryWidget->SetDefaultRenderer ( renderer );
  m_summaryWidget->SetCurrentRenderer ( renderer );

  /* ===================
   *  Start interaction
   * =================== */

  renderWindow->SetLineSmoothing(true);
  renderWindow->SetWindowName( regime == Regime_Full ? "Accessory Graph" : "Adjacency Graph" );
  //
  graphItem->StartLayoutAnimation( m_pWidget->GetInteractor() );
  //
  m_pWidget->GetInteractor()->Initialize();
  m_pWidget->resize(400, 400);
  m_pWidget->show();

  // Set callback on rendering
  m_pWidget->GetRenderWindow()->AddObserver(vtkCommand::RenderEvent, this, &exeFeatures_TopoGraph::RenderEventCallback);
}

//! Renders topology graph in the requested regime.
//! \param shape         [in] target shape.
//! \param selectedFaces [in] selected faces.
//! \param regime        [in] regime of interest.
//! \param leafType      [in] target leaf type for FULL regime.
void exeFeatures_TopoGraph::Render(const TopoDS_Shape&               shape,
                                   const TopTools_IndexedMapOfShape& selectedFaces,
                                   const Regime                      regime,
                                   const TopAbs_ShapeEnum            leafType)
{
  // Populate graph data from topology graph
  vtkSmartPointer<vtkGraph> graph = this->convertToGraph(shape, selectedFaces, regime, leafType);

  // Render VTK graph
  this->Render(graph, shape, regime);
}

//! Renders topology graph.
//! \param shape    [in] target shape.
//! \param leafType [in] target leaf type.
void exeFeatures_TopoGraph::RenderFull(const TopoDS_Shape&    shape,
                                       const TopAbs_ShapeEnum leafType)
{
  this->Render(shape, TopTools_IndexedMapOfShape(), Regime_Full, leafType);
}

//! Renders AA graph.
//! \param shape         [in] target shape.
//! \param selectedFaces [in] selected faces.
void exeFeatures_TopoGraph::RenderAdjacency(const TopoDS_Shape&               shape,
                                            const TopTools_IndexedMapOfShape& selectedFaces)
{
  this->Render(shape, selectedFaces, Regime_AAG, TopAbs_SHAPE);
}

//! Builds one or another graph (depending on the desired regime).
//! \param shape         [in] master model.
//! \param selectedFaces [in] optional selected faces.
//! \param regime        [in] desired regime.
//! \param leafType      [in] leaf type for FULL regime.
//! \return graph instance.
vtkSmartPointer<vtkGraph>
  exeFeatures_TopoGraph::convertToGraph(const TopoDS_Shape&               shape,
                                        const TopTools_IndexedMapOfShape& selectedFaces,
                                        const Regime                      regime,
                                        const TopAbs_ShapeEnum            leafType)
{
  vtkSmartPointer<vtkGraph> result;
  //
  if ( regime == Regime_Full )
  {
    result = vtkSmartPointer<vtkMutableDirectedGraph>::New();
    vtkMutableDirectedGraph* directed_result = dynamic_cast<vtkMutableDirectedGraph*>( result.GetPointer() );

    // Array for groups
    vtkNew<vtkStringArray> groupArr;
    groupArr->SetNumberOfComponents(1);
    groupArr->SetName(ARRNAME_GROUP);

    // Array for vertex labels
    vtkNew<vtkStringArray> labelArr;
    labelArr->SetNumberOfComponents(1);
    labelArr->SetName(ARRNAME_LABELS);

    // Create VTK data set for graph data
    const vtkIdType root_vid = directed_result->AddVertex();
    TopTools_DataMapOfShapeInteger M;
    M.Bind(shape, root_vid);
    //
    labelArr->InsertNextValue( asiAlgo_Utils::ShapeAddrWithPrefix(shape).c_str() );
    //
    if ( shape.ShapeType() == TopAbs_COMPOUND )
      groupArr->InsertNextValue(ARRNAME_GROUP_COMPOUND);
    else if ( shape.ShapeType() == TopAbs_FACE )
      groupArr->InsertNextValue(ARRNAME_GROUP_FACE);
    else if ( shape.ShapeType() == TopAbs_WIRE )
      groupArr->InsertNextValue(ARRNAME_GROUP_WIRE);
    else if ( shape.ShapeType() == TopAbs_EDGE )
      groupArr->InsertNextValue(ARRNAME_GROUP_EDGE);
    else if ( shape.ShapeType() == TopAbs_VERTEX )
      groupArr->InsertNextValue(ARRNAME_GROUP_VERTEX);
    else
      groupArr->InsertNextValue(ARRNAME_GROUP_ORDINARY);
    //
    this->buildRecursively(shape, root_vid, leafType, directed_result, labelArr.GetPointer(), groupArr.GetPointer(), M);

    // Set arrays
    result->GetVertexData()->AddArray( labelArr.GetPointer() );
    result->GetVertexData()->AddArray( groupArr.GetPointer() );
  }
  else if ( regime == Regime_AAG )
  {
    m_aag = new feature_aag(shape, selectedFaces);
    vtkSmartPointer<vtkMutableUndirectedGraph> undirected = exeFeatures_AAGAdaptor::Convert(m_aag);
    result = undirected;
  }
  else
    Standard_ProgramError::Raise("Unexpected regime for graph visualization");

  return result;
}

//! Builds data structures for visualization recursively.
//! \param rootShape     [in]     root shape.
//! \param rootId        [in]     ID of the root vertex.
//! \param leafType      [in]     topological type for leafs.
//! \param pDS           [in/out] data structure being filled.
//! \param pLabelArr     [in/out] array for labels associated with vertices.
//! \param pGroupArr     [in/out] array for vertex groups.
//! \param shapeVertices [in/out] map of shapes against their registered graph vertices.
void exeFeatures_TopoGraph::buildRecursively(const TopoDS_Shape&             rootShape,
                                             const vtkIdType                 rootId,
                                             const TopAbs_ShapeEnum          leafType,
                                             vtkMutableDirectedGraph*        pDS,
                                             vtkStringArray*                 pLabelArr,
                                             vtkStringArray*                 pGroupArr,
                                             TopTools_DataMapOfShapeInteger& shapeVertices)
{
  // Check if it is time to stop
  if ( rootShape.ShapeType() == leafType )
    return;

  // Iterate over the sub-shape
  for ( TopoDS_Iterator it(rootShape, 0, 0); it.More(); it.Next() )
  {
    const TopoDS_Shape& subShape = it.Value();

    vtkIdType childId;
    if ( shapeVertices.IsBound(subShape) )
    {
      childId = shapeVertices.Find(subShape);
    }
    else
    {
      childId = pDS->AddVertex();
      shapeVertices.Bind(subShape, childId);

      if ( pLabelArr )
        pLabelArr->InsertNextValue( asiAlgo_Utils::ShapeAddrWithPrefix(subShape).c_str() );

      if ( pGroupArr )
      {
        if ( subShape.ShapeType() == TopAbs_COMPOUND )
          pGroupArr->InsertNextValue(ARRNAME_GROUP_COMPOUND);
        else if ( subShape.ShapeType() == TopAbs_FACE )
          pGroupArr->InsertNextValue(ARRNAME_GROUP_FACE);
        else if ( subShape.ShapeType() == TopAbs_WIRE )
          pGroupArr->InsertNextValue(ARRNAME_GROUP_WIRE);
        else if ( subShape.ShapeType() == TopAbs_EDGE )
          pGroupArr->InsertNextValue(ARRNAME_GROUP_EDGE);
        else if ( subShape.ShapeType() == TopAbs_VERTEX )
          pGroupArr->InsertNextValue(ARRNAME_GROUP_VERTEX);
        else
          pGroupArr->InsertNextValue(ARRNAME_GROUP_ORDINARY);
      }
    }
    //
    pDS->AddEdge(rootId, childId);
    //
    this->buildRecursively(subShape, childId, leafType, pDS, pLabelArr, pGroupArr, shapeVertices);
  }
}

//! Callback to adjust text widgets.
void exeFeatures_TopoGraph::RenderEventCallback()
{
  if ( !m_textWidget->GetEnabled() )
    m_textWidget->On();

  if ( !m_summaryWidget->GetEnabled() )
    m_summaryWidget->On();
}

//! Reaction on closing the viewer.
void exeFeatures_TopoGraph::onViewerClosed()
{
  // NOTE: the important point is to remove widget after all items which
  //       may listen to it
  m_textWidget->Delete();
  m_summaryWidget->Delete();

  delete m_pWidget;
  delete this;
}

//! Reaction on vertex picking.
//! \param fid [in] face ID.
//! \param vid [in] vertex ID.
void exeFeatures_TopoGraph::onVertexPicked(const int fid, const vtkIdType asiVisu_NotUsed(vid))
{
  if ( m_aag.IsNull() )
    return;

  if ( exeFeatures_CommonFacilities::Instance()->Prs.Part )
  {
    // Get face from graph vertex
    const TopoDS_Face& F = m_aag->GetFace(fid);

    // Highlight in the main viewer
    TopTools_IndexedMapOfShape selected;
    selected.Add(F);
    //
    asiasiEngine_Part::HighlightSubShapes(selected);
  }
}
