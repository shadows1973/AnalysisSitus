//-----------------------------------------------------------------------------
// Created on: 23 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2012, Andrey Voevodin
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

#ifndef asiVisu_IVShapeNodeInfo_h
#define asiVisu_IVShapeNodeInfo_h

// A-Situs includes
#include <asiVisu_NodeInfo.h>

//! This is a sub-class for generic asiVisu_NodeInfo. Use this key to
//! distinguish between part presentations and other presentations bounded
//! to Active Data Nodes.
class asiVisu_IVShapeNodeInfo : public asiVisu_NodeInfo
{
public:

  vtkTypeMacro(asiVisu_IVShapeNodeInfo, vtkObject);

  asiVisu_EXPORT static asiVisu_IVShapeNodeInfo*
    New();

public:

  asiVisu_EXPORT static vtkInformationObjectBaseKey*
    GetKey();

  asiVisu_EXPORT static asiVisu_IVShapeNodeInfo*
    Retrieve(vtkActor* actor);

  asiVisu_EXPORT static void
    Store(const ActAPI_DataObjectId& nodeId,
          vtkActor*                  actor);

private:

  asiVisu_IVShapeNodeInfo();
  asiVisu_IVShapeNodeInfo(const asiVisu_IVShapeNodeInfo&);
  asiVisu_IVShapeNodeInfo& operator=(const asiVisu_IVShapeNodeInfo&);
  ~asiVisu_IVShapeNodeInfo();

};

#endif
