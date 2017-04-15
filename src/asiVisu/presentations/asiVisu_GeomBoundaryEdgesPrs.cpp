//-----------------------------------------------------------------------------
// Created on: 22 March 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_GeomBoundaryEdgesPrs.h>

// asiVisu includes
#include <asiVisu_ShapeDataProvider.h>
#include <asiVisu_ShapePipeline.h>

// VTK includes
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkProperty.h>

//! Creates a Presentation object for the passed Node.
//! \param N [in] Node to create a Presentation for.
asiVisu_GeomBoundaryEdgesPrs::asiVisu_GeomBoundaryEdgesPrs(const Handle(ActAPI_INode)& N) : asiVisu_Prs(N)
{
  // Create Data Providers
  Handle(asiVisu_ShapeDataProvider) dp_convex =
    new asiVisu_ShapeDataProvider( N->GetId(),
                                   ActParamStream() << N->Parameter(asiData_BoundaryEdgesNode::PID_Green) );
  //
  Handle(asiVisu_ShapeDataProvider) dp_concave =
    new asiVisu_ShapeDataProvider( N->GetId(),
                                   ActParamStream() << N->Parameter(asiData_BoundaryEdgesNode::PID_Red) );
  //
  Handle(asiVisu_ShapeDataProvider) dp_undefined =
    new asiVisu_ShapeDataProvider( N->GetId(),
                                   ActParamStream() << N->Parameter(asiData_BoundaryEdgesNode::PID_Ordinary) );

  //---------------------------------------------------------------------------
  // Pipeline for convex edges
  Handle(asiVisu_ShapePipeline) pl_convex = new asiVisu_ShapePipeline(false);
  //
  this->addPipeline        ( Pipeline_Convex, pl_convex );
  this->assignDataProvider ( Pipeline_Convex, dp_convex );
  //
  pl_convex->Actor()->GetProperty()->SetLineWidth(2.0f);
  pl_convex->Actor()->GetProperty()->SetColor(0.1, 0.7, 0.0);
  pl_convex->GetDisplayModeFilter()->SetDisplayMode(DisplayMode_Wireframe);

  //---------------------------------------------------------------------------
  // Pipeline for concave edges
  Handle(asiVisu_ShapePipeline) pl_concave = new asiVisu_ShapePipeline(false);
  //
  this->addPipeline        ( Pipeline_Concave, pl_concave );
  this->assignDataProvider ( Pipeline_Concave, dp_concave );
  //
  pl_concave->Actor()->GetProperty()->SetLineWidth(2.0f);
  pl_concave->Actor()->GetProperty()->SetColor(0.7, 0.0, 0.0);
  pl_concave->GetDisplayModeFilter()->SetDisplayMode(DisplayMode_Wireframe);

  //---------------------------------------------------------------------------
  // Pipeline for undefined edges
  Handle(asiVisu_ShapePipeline) pl_undefined = new asiVisu_ShapePipeline(false);
  //
  this->addPipeline        ( Pipeline_Undefined, pl_undefined );
  this->assignDataProvider ( Pipeline_Undefined, dp_undefined );
  //
  pl_undefined->Actor()->GetProperty()->SetLineWidth(2.0f);
  pl_undefined->Actor()->GetProperty()->SetColor(0.5, 0.5, 0.5);
  pl_undefined->GetDisplayModeFilter()->SetDisplayMode(DisplayMode_Wireframe);

  //---------------------------------------------------------------------------
  vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
}

//! Factory method for Presentation.
//! \param N [in] Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_GeomBoundaryEdgesPrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_GeomBoundaryEdgesPrs(N);
}

//! Returns true if the Presentation is visible, false -- otherwise.
//! \return true/false.
bool asiVisu_GeomBoundaryEdgesPrs::IsVisible() const
{
  return true;
}

//-----------------------------------------------------------------------------

//! Callback for initialization of Presentation pipelines.
void asiVisu_GeomBoundaryEdgesPrs::beforeInitPipelines()
{
}

//! Callback for initialization of Presentation pipelines.
void asiVisu_GeomBoundaryEdgesPrs::afterInitPipelines()
{
}

//! Callback for updating of Presentation pipelines invoked before the
//! kernel update routine starts.
void asiVisu_GeomBoundaryEdgesPrs::beforeUpdatePipelines() const
{
}

//! Callback for updating of Presentation pipelines invoked after the
//! kernel update routine completes.
void asiVisu_GeomBoundaryEdgesPrs::afterUpdatePipelines() const
{
}

//! Callback for highlighting.
void asiVisu_GeomBoundaryEdgesPrs::highlight(vtkRenderer*                  renderer,
                                             const asiVisu_PickResult&     pickRes,
                                             const asiVisu_SelectionNature selNature) const
{
  asiVisu_NotUsed(renderer);
  asiVisu_NotUsed(pickRes);
  asiVisu_NotUsed(selNature);
}

//! Callback for highlighting reset.
void asiVisu_GeomBoundaryEdgesPrs::unHighlight(vtkRenderer*                  renderer,
                                               const asiVisu_SelectionNature selNature) const
{
  asiVisu_NotUsed(renderer);
  asiVisu_NotUsed(selNature);
}

//! Callback for rendering.
void asiVisu_GeomBoundaryEdgesPrs::renderPipelines(vtkRenderer* renderer) const
{
  asiVisu_NotUsed(renderer);
}

//! Callback for de-rendering.
void asiVisu_GeomBoundaryEdgesPrs::deRenderPipelines(vtkRenderer* renderer) const
{
  asiVisu_NotUsed(renderer);
}
