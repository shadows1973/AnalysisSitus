//-----------------------------------------------------------------------------
// Created on: 09 December 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017 Sergey Slyadnev
// Code covered by the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_PointsVectorFilter.h>

// Visualization includes
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>

vtkStandardNewMacro(asiVisu_PointsVectorFilter);

//! Default constructor.
asiVisu_PointsVectorFilter::asiVisu_PointsVectorFilter()
{
  m_fMaxModulus =  1.0;
  m_fMinScalar  =  VTK_FLOAT_MAX;
  m_fMaxScalar  = -VTK_FLOAT_MAX;
}

//! Destructor.
asiVisu_PointsVectorFilter::~asiVisu_PointsVectorFilter()
{}

//! Specifies the applicable input types.
//! \param port [not used] connection port.
//! \param info [in/out] information object.
//! \return result code.
int asiVisu_PointsVectorFilter::FillInputPortInformation(int, vtkInformation* info)
{
  // This filter uses the vtkDataSet cell traversal methods so it
  // supports any data set type as input
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//! This method (called by superclass) performs conversion of OCCT
//! data structures to VTK polygonal representation.
//!
//! \param request      [in]  describes "what" algorithm should do. This is
//!                           typically just one key such as REQUEST_INFORMATION.
//! \param inputVector  [in]  inputs of the algorithm.
//! \param outputVector [out] outputs of the algorithm.
int asiVisu_PointsVectorFilter::RequestData(vtkInformation*,
                                            vtkInformationVector** inputVector,
                                            vtkInformationVector*  outputVector)
{
  // Get input and output data sets
  vtkDataSet*  input  = vtkDataSet::GetData(inputVector[0]);
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  const vtkIdType nbCells  = input->GetNumberOfCells();
  const vtkIdType nbPoints = input->GetNumberOfPoints();

  // Skip execution if there is no input topology / geometry
  if ( nbCells < 1 || nbPoints < 1 )
  {
    vtkDebugMacro("No points to process");
    return 1;
  }

  // Skip execution if there are no vectors in the field
  if ( m_normals.IsNull() || m_normals->GetNumberOfElements() < 1 )
  {
    m_fMaxScalar = m_fMinScalar = 0.0;
    vtkDebugMacro("No vectors to process");
    return 1;
  }

  // Reset min & max scalar values
  m_fMaxScalar = -VTK_FLOAT_MAX;
  m_fMinScalar =  VTK_FLOAT_MAX;

  // Allocate working space for the new and old cell point lists
  vtkSmartPointer<vtkIdList> oldPointIds = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdList> newPointIds = vtkSmartPointer<vtkIdList>::New();
  //
  oldPointIds->Allocate(VTK_CELL_SIZE);
  newPointIds->Allocate(VTK_CELL_SIZE);

  // Allocate space needed for the output cells
  output->Allocate(nbCells);

  // Allocate space for the new collection of points
  vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
  newPoints->Allocate(nbPoints, nbPoints);

  // Allocate space for data associated with the new set of points
  vtkPointData* outputPD = output->GetPointData();

  // Allocate space for data associated with the new set of cells
  vtkCellData* inputCD  = input->GetCellData();
  vtkCellData* outputCD = output->GetCellData();
  //
  outputCD->CopyAllocate(inputCD, nbCells, nbCells);

  // Allocate an array for vectors
  vtkSmartPointer<vtkDoubleArray>
    newCellVectors = asiVisu_Utils::InitDoubleVectorArray(ARRNAME_POINTCLOUD_VECTORS);

  // Traverse all cells
  for ( vtkIdType cellId = 0; cellId < nbCells; ++cellId )
  {
    // Get the list of points for this cell
    input->GetCellPoints(cellId, oldPointIds);

    // Create new points for this cell
    newPointIds->Reset();

    // Access scalar value
    float nx, ny, nz;
    m_normals->GetElement( (int) cellId, nx, ny, nz );
    asiVisu_VectorTuple aVecTuple(nx, ny, nz);
    this->adjustMinMax(aVecTuple);

    vtkIdType anOldPid = oldPointIds->GetId(0);
    double aCurrentCoords[3];
    input->GetPoint(anOldPid, aCurrentCoords);
    vtkIdType aNewPntPid = newPoints->InsertNextPoint(aCurrentCoords);
    newPointIds->InsertId(0, aNewPntPid);

    // Store the new cell in the output as vertex
    vtkIdType aNewCellPid = output->InsertNextCell(VTK_VERTEX, newPointIds);

    // Associate scalar with cell data
    double aVecCoords[3] = {aVecTuple.F[0], aVecTuple.F[1], aVecTuple.F[2]};
    newCellVectors->InsertTypedTuple(aNewCellPid, aVecCoords);
  }

  // Loop over all vectors calibrating their magnitudes by the maximum one
  for ( vtkIdType vecId = 0; vecId < newCellVectors->GetNumberOfTuples(); ++vecId )
  {
    double* aVecCoords = newCellVectors->GetTuple(vecId);

    double aVecModulus = 0.0;
    for ( int k = 0; k < 3; k++ )
      aVecModulus += aVecCoords[k] * aVecCoords[k];
    aVecModulus = Sqrt(aVecModulus);

    double aModFactor = (m_fMaxScalar ? m_fMaxModulus * (aVecModulus / m_fMaxScalar) : 0.0);
    for ( int k = 0; k < 3; k++ )
      aVecCoords[k] *= aModFactor;

    newCellVectors->SetTuple(vecId, aVecCoords);
  }

  // Set vectors
  outputPD->SetScalars(newCellVectors);

  // Store the new set of points in the output
  output->SetPoints(newPoints);

  // Avoid keeping extra memory around
  output->Squeeze();

  return 1;
}

//! Adjusts min & max scalar values against the passed vectorial data.
//! \param vecTuple [in] vectorial data.
void asiVisu_PointsVectorFilter::adjustMinMax(const asiVisu_VectorTuple& vecTuple)
{
  double aVecModulus = 0.0;
  for ( int k = 0; k < 3; k++ )
    aVecModulus += vecTuple.F[k] * vecTuple.F[k];
  aVecModulus = Sqrt(aVecModulus);

  if ( aVecModulus > m_fMaxScalar )
    m_fMaxScalar = aVecModulus;
  if ( aVecModulus < m_fMinScalar )
    m_fMinScalar = aVecModulus;
}
