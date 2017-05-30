//-----------------------------------------------------------------------------
// Created on: 15 April 2017
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiVisu_ShapePipeline_h
#define asiVisu_ShapePipeline_h

// asiVisu includes
#include <asiVisu_DisplayModeFilter.h>
#include <asiVisu_Pipeline.h>
#include <asiVisu_Selection.h>
#include <asiVisu_ShapeRobustSource.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// VTK includes
#include <vtkPolyDataNormals.h>

//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(Shape, asiVisu_Pipeline)

//! Visualization pipeline for OCCT shapes which are not stored in the Part
//! Node. Hence the visualization process here is not sophisticated and
//! less services are provided (e.g. no interaction at all).
class asiVisu_ShapePipeline : public asiVisu_Pipeline
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_ShapePipeline, asiVisu_Pipeline)

public:

  asiVisu_EXPORT
    asiVisu_ShapePipeline(const bool isScalarMode = true);

public:

  asiVisu_EXPORT void
    SetDiagnosticTools(ActAPI_ProgressEntry progress,
                       ActAPI_PlotterEntry  plotter);

  asiVisu_EXPORT virtual void
    SetInput(const Handle(asiVisu_DataProvider)& dataProvider);

public:

  //! \return data source.
  const vtkSmartPointer<asiVisu_ShapeRobustSource>& GetSource() const
  {
    return m_source;
  }

  //! \return normals calculation filter.
  const vtkSmartPointer<vtkPolyDataNormals>& GetNormalsFilter() const
  {
    return m_normalsFilter;
  }

  //! \return display mode filter.
  const vtkSmartPointer<asiVisu_DisplayModeFilter>& GetDisplayModeFilter() const
  {
    return m_dmFilter;
  }

private:

  virtual void callback_add_to_renderer      (vtkRenderer* renderer);
  virtual void callback_remove_from_renderer (vtkRenderer* renderer);
  virtual void callback_update               ();

private:

  //! Copying prohibited.
  asiVisu_ShapePipeline(const asiVisu_ShapePipeline&);

  //! Assignment prohibited.
  asiVisu_ShapePipeline& operator=(const asiVisu_ShapePipeline&);

protected:

  //! Inficates whether color scalars are used or not.
  bool m_bIsScalarModeOn;

  //! Technical flag indicating whether a GL-mapper is initialized with the
  //! custom color scheme or not.
  bool m_bMapperColorsSet;

  //! Data source.
  vtkSmartPointer<asiVisu_ShapeRobustSource> m_source;

  //! Filter for normals.
  vtkSmartPointer<vtkPolyDataNormals> m_normalsFilter;

  //! Display mode filter.
  vtkSmartPointer<asiVisu_DisplayModeFilter> m_dmFilter;

  //! Progress notifier.
  ActAPI_ProgressEntry m_progress;

  //! Imperative plotter.
  ActAPI_PlotterEntry m_plotter;

};

#endif
