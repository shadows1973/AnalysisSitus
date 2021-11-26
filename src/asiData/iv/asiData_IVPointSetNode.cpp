//-----------------------------------------------------------------------------
// Created on: 08 April 2016
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

// Own include
#include <asiData_IVPointSetNode.h>

// asiAlgo includes
#include <asiAlgo_PointCloudUtils.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_IVPointSetNode::asiData_IVPointSetNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,      PID_Name);
  REGISTER_PARAMETER(RealArray, PID_Geometry);
  REGISTER_PARAMETER(Selection, PID_Filter);
  REGISTER_PARAMETER(Group,     PID_GroupPrs);
  REGISTER_PARAMETER(Bool,      PID_HasColor);
  REGISTER_PARAMETER(Int,       PID_Color);
  REGISTER_PARAMETER(Real,      PID_PointSize);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_IVPointSetNode::Instance()
{
  return new asiData_IVPointSetNode();
}

//! Performs initial actions required to make Node WELL-FORMED.
void asiData_IVPointSetNode::Init()
{
  // Initialize name Parameter
  this->InitParameter (PID_Name,      "Name",         "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_GroupPrs,  "Presentation", "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_HasColor,  "Colorized",    "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_Color,     "Color",        "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_PointSize, "Point size",   "",               ParameterFlag_IsVisible, true);
  //
  this->SetPoints    (nullptr);
  this->SetFilter    (nullptr);
  this->SetPointSize (4);
  this->SetHasColor  (false);
  this->SetColor     (16711680);
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_IVPointSetNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param[in] name the name to set.
void asiData_IVPointSetNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------
// Handy API
//-----------------------------------------------------------------------------

//! \return stored point cloud.
Handle(asiAlgo_BaseCloud<double>) asiData_IVPointSetNode::GetPoints() const
{
  Handle(TColStd_HArray1OfReal)
    coords = ActParamTool::AsRealArray( this->Parameter(PID_Geometry) )->GetArray();
  //
  return asiAlgo_PointCloudUtils::AsCloudd(coords);
}

//! Sets point cloud to store.
//! \param[in] points points to store.
void asiData_IVPointSetNode::SetPoints(const Handle(asiAlgo_BaseCloud<double>)& points)
{
  Handle(TColStd_HArray1OfReal) arr = asiAlgo_PointCloudUtils::AsRealArray(points);
  //
  ActParamTool::AsRealArray( this->Parameter(PID_Geometry) )->SetArray( points.IsNull() ? nullptr : arr );
}

//! \return persistent filter.
Handle(TColStd_HPackedMapOfInteger) asiData_IVPointSetNode::GetFilter() const
{
  return ActParamTool::AsSelection( this->Parameter(PID_Filter) )->GetMask();
}

//! Sets filter of indices.
//! \param[in] filter mask to store.
void asiData_IVPointSetNode::SetFilter(const Handle(TColStd_HPackedMapOfInteger)& filter)
{
  ActParamTool::AsSelection( this->Parameter(PID_Filter) )->SetMask(filter);
}

//! Sets the Boolean value indicating whether the color Parameter of this
//! Data Node is in force.
//! \param[in] hasColor value to set.
void asiData_IVPointSetNode::SetHasColor(const bool hasColor)
{
  ActParamTool::AsBool( this->Parameter(PID_HasColor) )->SetValue(hasColor);
}

//! Accessor for the value of the Boolean Parameter indicating whether the
//! Color Parameter of this Data Node is in force.
//! \return true/false.
bool asiData_IVPointSetNode::HasColor() const
{
  return ActParamTool::AsBool( this->Parameter(PID_HasColor) )->GetValue();
}

//! Sets color.
//! \param[in] color color to set.
void asiData_IVPointSetNode::SetColor(const int color)
{
  ActParamTool::AsInt( this->Parameter(PID_Color) )->SetValue(color);
}

//! Accessor for the stored color value.
//! \return color value.
int asiData_IVPointSetNode::GetColor() const
{
  return ActParamTool::AsInt( this->Parameter(PID_Color) )->GetValue();
}

//! \return the stored point size.
double asiData_IVPointSetNode::GetPointSize() const
{
  return ActParamTool::AsReal( this->Parameter(PID_PointSize) )->GetValue();
}

//! Sets the point size to store.
//! \param[in] sz the value to set.
void asiData_IVPointSetNode::SetPointSize(const double sz)
{
  ActParamTool::AsReal( this->Parameter(PID_PointSize) )->SetValue(sz);
}
