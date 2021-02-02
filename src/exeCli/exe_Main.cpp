//-----------------------------------------------------------------------------
// Created on: 07 November 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2015-present, Sergey Slyadnev
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

// exe includes
#include <exe_CommandWindow.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

// VTK init
#include <vtkAutoInit.h>

// Activate object factories
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkIOExportOpenGL2)
VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2)

//-----------------------------------------------------------------------------

#define EXE_LOAD_MODULE(name) \
{ \
  Handle(asiUI_BatchFacilities) __cf = asiUI_BatchFacilities::Instance();\
  \
  asiTcl_Plugin::Status status = asiTcl_Plugin::Load(__cf->Interp, __cf, name); \
  if ( status == asiTcl_Plugin::Status_Failed ) \
    __cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else if ( status == asiTcl_Plugin::Status_OK ) \
    __cf->Progress.SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

//-----------------------------------------------------------------------------

//! Application utilities.
namespace CliUtils
{
  Handle(exe_CommandQueue) Queue; //!< Command queue.
}

//-----------------------------------------------------------------------------
// Console thread
//-----------------------------------------------------------------------------

//! Working routine for console thread.
DWORD WINAPI Thread_Console(LPVOID)
{
  Handle(asiUI_BatchFacilities) cf = asiUI_BatchFacilities::Instance();

  // Create terminal.
  exe_CommandWindow ConsoleWindow(CliUtils::Queue, cf);
  if ( !ConsoleWindow.Create() )
    return 1;

  // Register console commands in global repository
  //RegisterConsoleCommands();

  // Start message loop.
  ConsoleWindow.StartMessageLoop();

  return 0;
}

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

//! main().
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  // Create command queue in the main thread
  if ( CliUtils::Queue.IsNull() )
    CliUtils::Queue = new exe_CommandQueue;

  // Load default commands.
  EXE_LOAD_MODULE("cmdMisc")
  EXE_LOAD_MODULE("cmdEngine")
  EXE_LOAD_MODULE("cmdRE")
  EXE_LOAD_MODULE("cmdDDF")
  EXE_LOAD_MODULE("cmdAsm")
  //
#ifdef USE_MOBIUS
  EXE_LOAD_MODULE("cmdMobius")
#endif

  // Create thread for Console
  HANDLE hConsoleThread = CreateThread(NULL, 0, Thread_Console, NULL, 0, NULL);
  if ( !hConsoleThread )
    ExitProcess(NULL);

  // Aray to store thread handles
  HANDLE hThreads[] = {/*hViewerThread,*/ hConsoleThread};

  // NOTICE: we pass FALSE here as we do not want to have the Viewer opened
  //         while the Console is closed and vice versa. Once such behaviour
  //         becomes acceptable, change the bWaitAll to TRUE, so this
  //         barrier will be passed only then ALL threads are signaling
  WaitForMultipleObjects(1, hThreads, FALSE, INFINITE);

  // Close all thread handles upon completion
  //CloseHandle(hViewerThread);
  CloseHandle(hConsoleThread);

  return 0; // Success.
}
