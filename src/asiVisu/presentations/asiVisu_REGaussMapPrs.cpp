//-----------------------------------------------------------------------------
// Created on: 25 April 2017
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_REGaussMapPrs.h>

// asiVisu includes
#include <asiVisu_REGaussMapAxisDataProvider.h>
#include <asiVisu_REGaussMapDataProvider.h>
#include <asiVisu_REGaussMapRefinedAxisDataProvider.h>
#include <asiVisu_Utils.h>
#include <asiVisu_VectorsPipeline.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkProperty.h>

//! Creates a Presentation object for the passed Node.
//! \param node [in] Node to create a Presentation for.
asiVisu_REGaussMapPrs::asiVisu_REGaussMapPrs(const Handle(ActAPI_INode)& node)
: asiVisu_Prs(node)
{
  Handle(asiData_REGaussMapNode)
    gauss_n = Handle(asiData_REGaussMapNode)::DownCast(node);

  // Create Data Provider for Gauss Map
  Handle(asiVisu_REGaussMapDataProvider)
    DP = new asiVisu_REGaussMapDataProvider(gauss_n);

  // Create Data Provider for axis vector
  Handle(asiVisu_REGaussMapAxisDataProvider)
    DPAxis = new asiVisu_REGaussMapAxisDataProvider(gauss_n);

  // Create Data Provider for refined axis vector
  Handle(asiVisu_REGaussMapRefinedAxisDataProvider)
    DPRefinedAxis = new asiVisu_REGaussMapRefinedAxisDataProvider(gauss_n);

  // Pipeline for vectors
  this->addPipeline        ( Pipeline_GaussMap, new asiVisu_VectorsPipeline );
  this->assignDataProvider ( Pipeline_GaussMap, DP );

  // Pipeline for main axis
  this->addPipeline        ( Pipeline_Axis, new asiVisu_VectorsPipeline );
  this->assignDataProvider ( Pipeline_Axis, DPAxis );

  // Pipeline for refined axis
  this->addPipeline        ( Pipeline_RefinedAxis, new asiVisu_VectorsPipeline );
  this->assignDataProvider ( Pipeline_RefinedAxis, DPRefinedAxis );

  // Adjust colors
  this->GetPipeline(Pipeline_Axis)->Mapper()->ScalarVisibilityOff();
  this->GetPipeline(Pipeline_Axis)->Actor()->GetProperty()->SetColor(1.0, 0.0, 0.0);
  //
  this->GetPipeline(Pipeline_RefinedAxis)->Mapper()->ScalarVisibilityOff();
  this->GetPipeline(Pipeline_RefinedAxis)->Actor()->GetProperty()->SetColor(0.0, 1.0, 0.0);
}

//! Factory method for Presentation.
//! \param node [in] Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_REGaussMapPrs::Instance(const Handle(ActAPI_INode)& node)
{
  return new asiVisu_REGaussMapPrs(node);
}

//! Returns true if the Presentation is visible, false -- otherwise.
//! \return true/false.
bool asiVisu_REGaussMapPrs::IsVisible() const
{
  return true;
}

//-----------------------------------------------------------------------------

//! Callback for initialization of Presentation pipelines.
void asiVisu_REGaussMapPrs::beforeInitPipelines()
{
  // Do nothing...
}

//! Callback for initialization of Presentation pipelines.
void asiVisu_REGaussMapPrs::afterInitPipelines()
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked before the
//! kernel update routine starts.
void asiVisu_REGaussMapPrs::beforeUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked after the
//! kernel update routine completes.
void asiVisu_REGaussMapPrs::afterUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for highlighting.
void asiVisu_REGaussMapPrs::highlight(vtkRenderer*                  renderer,
                                      const asiVisu_PickResult&     pickRes,
                                      const asiVisu_SelectionNature selNature) const
{
  asiVisu_NotUsed(renderer);
  asiVisu_NotUsed(pickRes);
  asiVisu_NotUsed(selNature);
}

//! Callback for highlighting reset.
void asiVisu_REGaussMapPrs::unHighlight(vtkRenderer*                  renderer,
                                        const asiVisu_SelectionNature selNature) const
{
  asiVisu_NotUsed(renderer);
  asiVisu_NotUsed(selNature);
}

//! Callback for rendering.
void asiVisu_REGaussMapPrs::renderPipelines(vtkRenderer* renderer) const
{
  asiVisu_NotUsed(renderer);
}

//! Callback for de-rendering.
void asiVisu_REGaussMapPrs::deRenderPipelines(vtkRenderer* renderer) const
{
  asiVisu_NotUsed(renderer);
}
