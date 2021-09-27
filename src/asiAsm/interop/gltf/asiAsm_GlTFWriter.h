/***************************************************************************
 *   Copyright (c) OPEN CASCADE SAS                                        *
 *                                                                         *
 *   This file is part of Open CASCADE Technology software library.        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 ***************************************************************************/

#pragma once

// glTF includes
#include <asiAsm_GlTFCSysConverter.h>
#include <asiAsm_GlTFXdeVisualStyle.h>
#include <asiAsm_GlTFIDataSourceProvider.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <TColStd_IndexedDataMapOfStringString.hxx>
#include <TColStd_MapOfAsciiString.hxx>
#include <TDF_LabelSequence.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopoDS_Edge.hxx>

// Include for shared_ptr
#include <memory>

//-----------------------------------------------------------------------------

// Forward declarations from the global namespace.
class TDocStd_Document;
class XCAFDoc_ShapeTool;

namespace asiAsm {
namespace xde {

// Forward declarations from the `asiAsm` namespace.
class gltf_MaterialMap;

//! Writes the passed XDE document to glTF/glb format. All B-rep shapes in the
//! document should have undergone meshing to get facets for export.
//!
//! The glTF format was defined and specified by the Khronos Group for the
//! efficient transfer of 3D content over networks. The core of glTF is a JSON
//! file that describes the structure and composition of a scene containing 3D
//! models. This class is forked from the open-source OpenCascade kernel
//! to serve as a simplistic glTF writer for colorized CAD parts and
//! assemblies. Here is why we have it here:
//!
//! 1) glTF is a format of choice because it has got the first-class support in
//!    Three.js. It is also a trendy format in the computer graphics community,
//!    with a broad range of software packages supporting it.
//!
//! 2) We do not need any textures or advanced materials. The only essential
//!    ingredients are the scene tree (for CAD assemblies), colors (for visual
//!    diagnostics), and facets groups (to have several triangles represent one
//!    CAD face). Therefore, this version of glTF writer is more lightweight.
//!
//! 3) The original version of glTF writer in OpenCascade is too coupled with the
//!    rest of OpenCascade kernel. This version is our attempt to make it more
//!    compact in its code base.
//! To know more about glTF format follow this: https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_001_Introduction.md
class gltf_Writer : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(gltf_Writer, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] filename the full path to the file to write.
  //! \param[in] isBinary the Boolean flag indicating whether to
  //!                     write into `glb` (all-in-one-file) format.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  gltf_EXPORT
    gltf_Writer(const TCollection_AsciiString& filename,
               const bool                     isBinary,
               ActAPI_ProgressEntry           progress = nullptr,
               ActAPI_PlotterEntry            plotter  = nullptr);

  //! Dtor.
  gltf_EXPORT virtual
    ~gltf_Writer();

public:

  //! Writes glTF file and associated binary file.
  //! Triangulation data should be precomputed within shapes.
  //! You can pass additional metadata as `fileInfo`. It will
  //! be expanded to the `extras` object under `asset` entity:
  //!
  //!  "asset": {
  //!    "generator": "XXX",
  //!    "version": "2.0",
  //!      "extras": {
  //!        "Author": "STEP Inspector",
  //!        "Organization": "YYY"
  //!      }
  //!  }
  //!
  //! \param[in] dataProvider       the input data source provider feeding glTF writer with scene structure and nodes content.
  //! \param[in] fileInfo           the map with file metadata to put into glTF header section.
  //!
  //! \return true in case of success, false -- otherwise.
  gltf_EXPORT virtual bool
    Perform(const Handle(gltf_IDataSourceProvider)& dataProvider,
            const TColStd_IndexedDataMapOfStringString& fileInfo = TColStd_IndexedDataMapOfStringString());

public:

  //! \return const reference to the transformation from OCCT to glTF
  //!         coordinate system.
  gltf_EXPORT const gltf_CSysConverter&
    CoordinateSystemConverter() const;

  //! \return non-const reference to the transformation from OCCT to glTF
  //!         coordinate system.
  gltf_EXPORT gltf_CSysConverter&
    ChangeCoordinateSystemConverter();

  //! Set the coordinate system converter to use.
  //! \param[in] converter the converter to set.
  gltf_EXPORT void
    SetCoordinateSystemConverter(const gltf_CSysConverter& converter);

  //! \return the Boolean flag indicating whether to write into binary glTF
  //!         format (.glb), specified within class constructor.
  gltf_EXPORT bool
    IsBinary() const;

  //! \return the preferred transformation format for writing into glTF file.
  gltf_EXPORT gltf_WriterTrsfFormat
    TransformationFormat() const;

  //! Sets the preferred transformation format for writing into glTF file.
  //! \param[in] fmt the format to set.
  gltf_EXPORT void
    SetTransformationFormat(const gltf_WriterTrsfFormat fmt);

  //! \return true to export UV coordinates even if there are no mapped texture.
  gltf_EXPORT bool
    IsForcedUVExport() const;

  //! Sets a Boolean flag to export UV coordinates even if there are no mapped textures.
  gltf_EXPORT void
    SetForcedUVExport(const bool toForce);

  //! \return the default material definition to be used for nodes with only color defined.
  gltf_EXPORT const gltf_XdeVisualStyle&
    DefaultStyle() const;

  //! Sets the default material definition to be used for nodes with
  //! only color defined.
  //! \param[in] style the style to set as a default one.
  gltf_EXPORT void
    SetDefaultStyle(const gltf_XdeVisualStyle& style);

protected:

  //! Writes binary data file with triangulation data.
  //! Triangulation data should be precomputed within shapes.
  //! \return true in case of success, false -- otherwise.
  gltf_EXPORT virtual bool
    writeBinData();

  //! Writes JSON file with glTF structure (should be called after `writeBinData()`).
  //! \param[in] fileInfo  extras for the JSON's header section.
  //! \return true in case of success, false -- otherwise.
  gltf_EXPORT virtual bool
    writeJson(const TColStd_IndexedDataMapOfStringString& fileInfo);

protected:

  //! Writes mesh nodes into binary file.
  //! \param[out]    binFile    output file to write into.
  //! \param[in,out] accessorNb last accessor index.
  gltf_EXPORT virtual void
    writeBinDataNodes(std::ostream&            binFile,
                      int&                     accessorNb) const;

  //! Writes mesh normals into binary file.
  //! \param[out]    binFile    output file to write into.
  //! \param[in,out] accessorNb last accessor index.
  gltf_EXPORT virtual void
    writeBinDataNormals(std::ostream&      binFile,
                        int&               accessorNb) const;

  //! Writes mesh texture UV coordinates into binary file.
  //! \param[out]    binFile    output file to write into.
  //! \param[in,out] accessorNb last accessor index.
  gltf_EXPORT virtual void
    writeBinDataTextCoords(std::ostream&            binFile,
                           int&                     accessorNb) const;

  gltf_EXPORT virtual void writeBinDataNodalColors(std::ostream& binFile,
                                                   int& accessorNb) const;

  //! Writes meshes indexes into binary file.
  //! \param[out]    gltfFace   glTF face definition.
  //! \param[out]    binFile    output file to write into.
  //! \param[in]     faceIter   current face to write.
  //! \param[in,out] accessorNb last accessor index.
  gltf_EXPORT virtual void
    writeBinDataIndices(std::ostream&            binFile,
                        int&                     accessorNb);

protected:

  //! Writes bufferView for vertex positions within "accessors" section.
  //! \param[in] gltfFace face definition to write
  gltf_EXPORT virtual void
    writeNodalPositions(const gltf_Primitive& gltfPrm);

  //! Writes bufferView for vertex normals within "accessors" section.
  //! \param[in] gltfFace face definition to write
  gltf_EXPORT virtual void
    writeNodalNormals(const gltf_Primitive& gltfPrm);

  //! Writes bufferView for vertex texture coordinates within "accessors" section
  //! \param[in] gltfFace face definition to write
  gltf_EXPORT virtual void
    writeNodalTextCoords (const gltf_Primitive& gltfPrm);

  //! Writes bufferView for vertex colors within "accessors" section
  //! \param[in] gltfFace face definition to write
  gltf_EXPORT virtual void
    writeNodalColors(const gltf_Primitive& gltfPrm);

  //! Writes bufferView for triangle indexes within "accessors" section.
  //! \param[in] gltfFace face definition to write
  gltf_EXPORT virtual void
    writeNodalIndices(const gltf_Primitive& gltfPrm);

protected:

  //! Writes "accessors" section.
  //! \param[in] scNodeMap ordered map of scene nodes
  gltf_EXPORT virtual void
    writeAccessors();

  //! Writes "animations" section (reserved).
  gltf_EXPORT virtual void
    writeAnimations();

  //! Writes "asset" section.
  //! \param[in] fileInfo optional metadata to write into file header.
  gltf_EXPORT virtual void
    writeAsset(const TColStd_IndexedDataMapOfStringString& fileInfo);

  //! Writes "bufferViews" section.
  //! \param[in] binDataBufferId index of binary buffer with vertex data.
  gltf_EXPORT virtual void
    writeBufferViews(const int binDataBufferId);

  //! Writes "buffers" section.
  gltf_EXPORT virtual void
    writeBuffers();

  //! Writes extensions (reserved).
  gltf_EXPORT virtual void
    writeExtensions();

  //! Writes "images" section.
  //! \param[out] materialMap map of materials, filled with image files used by textures.
  gltf_EXPORT virtual void
    writeImages(gltf_MaterialMap&        materialMap);

  //! Writes "materials" section.
  //! \param[out] materialMap map of materials.
  gltf_EXPORT virtual void
    writeMaterials(gltf_MaterialMap&        materialMap);

  //! Writes "meshes" section.
  //! \param[in] materialMap map of materials.
  gltf_EXPORT virtual void
    writeMeshes(const gltf_MaterialMap&  materialMap);

  //! Writes "nodes" section for the scene nodes.
  gltf_EXPORT virtual void
    writeNodes();

  //! Writes "samplers" section.
  //! \param[in] materialMap map of materials.
  gltf_EXPORT virtual void
    writeSamplers(const gltf_MaterialMap& materialMap);

  //! Writes "scene" section.
  //! \param[in] defSceneId index of default scene (0).
  gltf_EXPORT virtual void
    writeScene(const int defSceneId);

  //! Writes "scenes" section.
  //! \param[in] scRootIds sequence of scene nodes pointing to root shapes.
  gltf_EXPORT virtual void
    writeScenes(/*const NCollection_Sequence<int>& scRootIds*/);

  //! Writes "skins" section (reserved).
  gltf_EXPORT virtual void
    writeSkins();

  //! Writes "textures" section.
  //! \param[out] materialMap map of materials, filled with textures.
  gltf_EXPORT virtual void
    writeTextures(gltf_MaterialMap&        materialMap);

protected:

  typedef std::shared_ptr<gltf_JsonSerializer> gltf_JsonSerializerPtr;

  Handle(XCAFDoc_ShapeTool) m_shapeTool;         //!< Shape Tool of XCAF.
  TCollection_AsciiString   m_filename;          //!< Output glTF file.
  TCollection_AsciiString   m_binFilenameFull;   //!< Output file with binary data (full path).
  TCollection_AsciiString   m_binFilenameShort;  //!< Output file with binary data (short path).
  gltf_WriterTrsfFormat     m_trsfFormat;        //!< Transformation format to use in glTF file.
  bool                      m_bIsBinary;         //!< Flag to write into binary glTF format (.glb).
  bool                      m_bIsForcedUVExport; //!< Export UV coordinates even if there are no mapped texture.
  gltf_CSysConverter        m_CSTrsf;            //!< Transformation from OCCT to glTF coordinate system.
  gltf_XdeVisualStyle       m_defaultStyle;      //!< Default material definition to be used for nodes with only color defined.
  gltf_JsonSerializerPtr    m_jsonWriter;        //!< JSON writer.

  gltf_BufferView           m_buffViewNodalPos;       //!< Current buffer view with nodes positions.
  gltf_BufferView           m_buffViewNodalNorm;      //!< Current buffer view with nodes normals.
  gltf_BufferView           m_buffViewNodalTextCoord; //!< Current buffer view with nodes UV coordinates.
  gltf_BufferView           m_buffViewNodalColor;     //!< Current buffer view with nodes UV coordinates.
  gltf_BufferView           m_buffViewIndices;        //!< Current buffer view with triangulation indexes.

  int64_t                   m_binDataLen64;      //!< Length of binary file.

  Handle(gltf_IDataSourceProvider) m_dataProvider;
};
} // xde
} // asiAsm

