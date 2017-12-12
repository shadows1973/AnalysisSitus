//-----------------------------------------------------------------------------
// Created on: 24 August 2017
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
//    * Neither the name of Sergey Slyadnev nor the
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

// cmdEngine includes
#include <cmdEngine.h>

// asiVisu includes
#include <asiVisu_Utils.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

//-----------------------------------------------------------------------------

int ENGINE_Erase(const Handle(asiTcl_Interp)& interp,
                 int                          argc,
                 const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find node
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[1]);
  //
  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
    return TCL_OK;
  }

  // Hide
  if ( cmdEngine::cf->ViewerPart->PrsMgr()->IsPresented(node) )
  {
    cmdEngine::cf->ViewerPart->PrsMgr()->DeRenderPresentation(node);
  }
  else
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no presentable object with name %1." << argv[1]);

  // Repaint
  cmdEngine::cf->ViewerPart->Repaint();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DOnly(const Handle(asiTcl_Interp)& interp,
                 int                          argc,
                 const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find node
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[1]);
  //
  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
    return TCL_OK;
  }

  // Display only
  cmdEngine::cf->ViewerPart->PrsMgr()->DeRenderAllPresentations();
  //
  if ( cmdEngine::cf->ViewerPart->PrsMgr()->IsPresented(node) )
  {
    cmdEngine::cf->ViewerPart->PrsMgr()->RenderPresentation(node);
  }
  else
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no presentable object with name %1." << argv[1]);

  // Repaint
  cmdEngine::cf->ViewerPart->Repaint();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_EraseAll(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  cmdEngine::cf->ViewerPart->PrsMgr()->DeRenderAllPresentations();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Clear(const Handle(asiTcl_Interp)& interp,
                 int                          argc,
                 const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Clear viewers
  cmdEngine::ClearViewers();

  // Clear data
  cmdEngine::model->Clear();

  // Update object browser
  cmdEngine::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Fit(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  asiVisu_Utils::ResetCamera( cmdEngine::cf->ViewerPart->PrsMgr()->GetRenderer(),
                              cmdEngine::cf->ViewerPart->PrsMgr()->PropsByTrihedron() );
  //
  asiVisu_Utils::AdjustCamera( cmdEngine::cf->ViewerPart->PrsMgr()->GetRenderer(),
                               cmdEngine::cf->ViewerPart->PrsMgr()->PropsByTrihedron() );
  //
  cmdEngine::cf->ViewerPart->Repaint();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdEngine::Commands_Viewer(const Handle(asiTcl_Interp)&      interp,
                                const Handle(Standard_Transient)& data)
{
  cmdEngine_NotUsed(data);
  //
  static const char* group = "cmdEngine";

//-------------------------------------------------------------------------//
  interp->AddCommand("erase",
    //
    "erase varName \n"
    "\t Hides object in viewer.",
    //
    __FILE__, group, ENGINE_Erase);

  //-------------------------------------------------------------------------//
  interp->AddCommand("donly",
    //
    "donly varName \n"
    "\t Shows only the given object in viewer.",
    //
    __FILE__, group, ENGINE_DOnly);

  //-------------------------------------------------------------------------//
  interp->AddCommand("erase-all",
    //
    "erase-all \n"
    "\t Erases all objects from the scene.",
    //
    __FILE__, group, ENGINE_EraseAll);

  //-------------------------------------------------------------------------//
  interp->AddCommand("clear",
    //
    "clear \n"
    "\t Cleans up project data.",
    //
    __FILE__, group, ENGINE_Clear);

  //-------------------------------------------------------------------------//
  interp->AddCommand("fit",
    //
    "fit \n"
    "\t Fits camera to the scene contents.",
    //
    __FILE__, group, ENGINE_Fit);
}
