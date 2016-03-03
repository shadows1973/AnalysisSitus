//-----------------------------------------------------------------------------
// Created on: 26 November 2015
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

#ifndef visu_display_mode_h
#define visu_display_mode_h

//! Enumerates display modes.
enum visu_display_mode
{
  DisplayMode_Undefined = 0x0000, //!< Undefined display mode.
  DisplayMode_Shading   = 0x0001, //!< For OCCT topology & meshes.
  DisplayMode_Wireframe = 0x0002, //!< For OCCT topology & meshes.
  DisplayMode_Shrink    = 0x0004  //!< For meshes only.
};

#endif
