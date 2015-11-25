//-----------------------------------------------------------------------------
// Created on: 25 September 2015
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://quaoar.su/blog/
//-----------------------------------------------------------------------------

#ifndef visu_shape_pipeline_h
#define visu_shape_pipeline_h

// A-Situs includes
#include <visu_data_provider.h>
#include <visu_pipeline.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>

// VTK includes
#include <vtkPolyDataAlgorithm.h>

//-----------------------------------------------------------------------------
// Data Provider
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(visu_shape_data_provider, visu_data_provider)

//! Data source for Shape pipeline.
class visu_shape_data_provider : public visu_data_provider
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI(visu_shape_data_provider, visu_data_provider)

public:

  virtual TopoDS_Shape
    GetShape() const = 0;

  virtual Handle(TColStd_HPackedMapOfInteger)
    GetSubShapes() const = 0;

  virtual bool
    HasPosition() const = 0;

  virtual bool
    HasPositionAndRotation() const = 0;

  virtual void
    GetPosition(double& thePosX,
                double& thePosY,
                double& thePosZ) const = 0;

  virtual void
    GetRotation(double& theAngleAroundX,
                double& theAngleAroundY,
                double& theAngleAroundZ) const = 0;
};

//-----------------------------------------------------------------------------
// Pipeline
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(visu_shape_pipeline, visu_pipeline)

//! Visualization pipeline for OCCT shapes.
class visu_shape_pipeline : public visu_pipeline
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI(visu_shape_pipeline, visu_pipeline)

public:

  ASitus_EXPORT
    visu_shape_pipeline(const bool isOCCTColorScheme = true,
                        const bool isBound2Node      = true,
                        const bool isSecondary       = false);

public:

  ASitus_EXPORT virtual void
    SetInput(const Handle(visu_data_provider)& theDataProvider);

public:

  ASitus_EXPORT bool IsShadingMode() const;
  ASitus_EXPORT void ShadingModeOn();

  ASitus_EXPORT bool IsWireframeMode() const;
  ASitus_EXPORT void WireframeModeOn();

  ASitus_EXPORT void VoidSubShapesOn();
  ASitus_EXPORT void VoidSubShapesOff();

  ASitus_EXPORT void SharedVerticesOn();
  ASitus_EXPORT void SharedVerticesOff();

  ASitus_EXPORT vtkPolyDataAlgorithm* DataSource() const;

private:

  virtual void addToRendererCallback      (vtkRenderer* theRenderer);
  virtual void removeFromRendererCallback (vtkRenderer* theRenderer);
  virtual void updateCallback             ();

private:

  //! Copying prohibited.
  visu_shape_pipeline(const visu_shape_pipeline&);

  //! Assignment prohibited.
  visu_shape_pipeline& operator=(const visu_shape_pipeline&);

protected:

  //! Internally used filters.
  enum FilterId
  {
    Filter_Source = 1, //!< Shape Data Source.
    Filter_SubShapes,  //!< Sub-shapes filter.
    Filter_DM,         //!< Display Mode filter.
    Filter_Normals     //!< Filter to generate normals for smooth shading.
  };

  //! Auxiliary map of internal filters by their correspondent IDs.
  typedef NCollection_DataMap< FilterId, vtkSmartPointer<vtkAlgorithm> > FilterMap;

protected:

  //! Map of internally used filters.
  FilterMap m_filterMap;

  //! Indicates whether the standard OCCT color mapping is used as a default
  //! color scheme for the visualized shapes.
  bool m_bOCCTColorScheme;

  //! Indicates whether this pipeline must propagate the corresponding Node
  //! ID to its actor via vtkInformation hook.
  bool m_bIsBound2Node;

  //! Indicates whether the pipeline is secondary or not.
  bool m_bIsSecondary;

  //! Technical flag indicating whether a GL-mapper is initialized with the
  //! custom color scheme or not.
  bool m_bMapperColorsSet;

  //! True if SHADING mode if turned ON, false -- otherwise. This flag is
  //! mutually exclusive with m_bWireframeMode.
  bool m_bShadingMode;

  //! True if WIREFRAME mode if turned ON, false -- otherwise. This flag is
  //! mutually exclusive with m_bShadingMode.
  bool m_bWireframeMode;

  //! Indicates whether sub-shapes filter is forced to block data regardless
  //! of the contents of Data Provider.
  bool m_bSubShapesVoid;

};

#endif
