//-----------------------------------------------------------------------------
// Created on: 14 May 2022 (*)
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

// cmdMisc includes
#include <cmdMisc.h>

// OpenCascade includes
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <TopExp_Explorer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <IntTools_EdgeFace.hxx>

#include <asiEngine_Part.h>

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          argc,
              const char**                 argv)
{
  TopoDS_Shape
    partShape = Handle(asiEngine_Model)::DownCast( interp->GetModel() )->GetPartNode()->GetShape();

  if ( argc < 2 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Pass face ID as the only argument.");
    return TCL_ERROR;
  }

  int faceId = atoi(argv[1]); // 1-based

  TopTools_IndexedMapOfShape M, ME;
  TopExp::MapShapes(partShape, TopAbs_FACE, M);
  TopExp::MapShapes(partShape, TopAbs_EDGE, ME);
  //
  TopoDS_Face face = TopoDS::Face( M.FindKey(faceId) );

  // Attempt to get the highlighted sub-shapes.
  TColStd_PackedMapOfInteger edgeIds;
  //
  if ( cmdMisc::cf && cmdMisc::cf->ViewerPart )
  {
    asiEngine_Part PartAPI( cmdMisc::model,
                            cmdMisc::cf->ViewerPart->PrsMgr(),
                            interp->GetProgress(),
                            interp->GetPlotter() );
    //
    PartAPI.GetHighlightedEdges(edgeIds);
  }
  //
  if ( edgeIds.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No edge selected.");
    return TCL_ERROR;
  }
  //
  TopoDS_Edge refEdge;
  BRepBuilderAPI_MakeWire mkWire;
  for ( TColStd_PackedMapOfInteger::Iterator eit(edgeIds); eit.More(); eit.Next() )
  {
    const int   eid  = eit.Key();
    TopoDS_Edge edge = TopoDS::Edge( ME.FindKey(eid) );

    mkWire.Add(edge);

    if ( refEdge.IsNull() )
      refEdge = edge;
  }
  //
  TopoDS_Wire W = mkWire.Wire();
  //
  interp->GetPlotter().REDRAW_SHAPE("W", W, Color_Green, 1., true);

  TopoDS_Face extendedFace;
  BRepLib::ExtendFace(face, 25, true, true, true, true, extendedFace);

  interp->GetPlotter().REDRAW_SHAPE("extendedFace", extendedFace);

  ShapeAnalysis_Edge edgeA = ShapeAnalysis_Edge();
  gp_Pnt iniPt = BRep_Tool::Pnt( edgeA.FirstVertex(refEdge) );

  Handle(Geom_Curve) curve;
  Standard_Real a, b;
  edgeA.Curve3d(refEdge, curve, a, b);
  gp_Pnt P;
  gp_Vec V1;
  curve->D1(0, P, V1);

  auto circle = gp_Circ(gp_Ax2(iniPt, V1.Reversed()), 25);
  auto profile_edge = BRepBuilderAPI_MakeEdge(circle).Edge();
  auto pipe = BRepOffsetAPI_MakePipe(W, profile_edge).Shape();
  //auto profile_wire = BRepBuilderAPI_MakeWire(profile_edge).Wire();
  //auto profile_face = BRepBuilderAPI_MakeFace(profile_wire).Face();

  interp->GetPlotter().REDRAW_SHAPE("pipe", pipe);

  BRepAlgoAPI_Section intersection(pipe, extendedFace);
  auto dbg = intersection.Shape();

  interp->GetPlotter().DRAW_SHAPE(intersection, Color_Red, 1., true, "intersection");

 /* for (TopExp_Explorer it(dbg, TopAbs_EDGE); it.More(); it.Next()) {
      auto checkEdge = TopoDS::Edge(it.Value());
      auto intersectionChecker = IntTools_EdgeFace();
      intersectionChecker.SetEdge(checkEdge);
      intersectionChecker.SetFace(face);
      intersectionChecker.Perform();
      if (intersectionChecker.CommonParts().IsEmpty()) {
          auto wireBuilder2 = BRepBuilderAPI_MakeWire();
          wireBuilder2.Add(checkEdge);
      }
  }*/

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int MISC_TestBottle(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  /* OpenCascade tutorial from
     https://dev.opencascade.org/doc/overview/html/occt__tutorial.html
   */

  Standard_Real myWidth = 10;
  interp->GetKeyValue(argc, argv, "w", myWidth);

  Standard_Real myHeight = 15;
  interp->GetKeyValue(argc, argv, "h", myHeight);

  Standard_Real myThickness = 5;
  interp->GetKeyValue(argc, argv, "t", myThickness);

  // Profile : Define Support Points
  gp_Pnt aPnt1(-myWidth / 2., 0, 0);        
  gp_Pnt aPnt2(-myWidth / 2., -myThickness / 4., 0);
  gp_Pnt aPnt3(0, -myThickness / 2., 0);
  gp_Pnt aPnt4(myWidth / 2., -myThickness / 4., 0);
  gp_Pnt aPnt5(myWidth / 2., 0, 0);
 
  // Profile : Define the Geometry
  Handle(Geom_TrimmedCurve) anArcOfCircle = GC_MakeArcOfCircle(aPnt2,aPnt3,aPnt4);
  Handle(Geom_TrimmedCurve) aSegment1 = GC_MakeSegment(aPnt1, aPnt2);
  Handle(Geom_TrimmedCurve) aSegment2 = GC_MakeSegment(aPnt4, aPnt5);
 
  // Profile : Define the Topology
  TopoDS_Edge anEdge1 = BRepBuilderAPI_MakeEdge(aSegment1);
  TopoDS_Edge anEdge2 = BRepBuilderAPI_MakeEdge(anArcOfCircle);
  TopoDS_Edge anEdge3 = BRepBuilderAPI_MakeEdge(aSegment2);
  TopoDS_Wire aWire  = BRepBuilderAPI_MakeWire(anEdge1, anEdge2, anEdge3);
 
  // Complete Profile
  gp_Ax1 xAxis = gp::OX();
  gp_Trsf aTrsf;
 
  aTrsf.SetMirror(xAxis);
  BRepBuilderAPI_Transform aBRepTrsf(aWire, aTrsf);
  TopoDS_Shape aMirroredShape = aBRepTrsf.Shape();
  TopoDS_Wire aMirroredWire = TopoDS::Wire(aMirroredShape);
 
  BRepBuilderAPI_MakeWire mkWire;
  mkWire.Add(aWire);
  mkWire.Add(aMirroredWire);
  TopoDS_Wire myWireProfile = mkWire.Wire();
 
  // Body : Prism the Profile
  TopoDS_Face myFaceProfile = BRepBuilderAPI_MakeFace(myWireProfile);
  gp_Vec aPrismVec(0, 0, myHeight);
  TopoDS_Shape myBody = BRepPrimAPI_MakePrism(myFaceProfile, aPrismVec);
 
  // Body : Apply Fillets
  BRepFilletAPI_MakeFillet mkFillet(myBody);
  TopExp_Explorer anEdgeExplorer(myBody, TopAbs_EDGE);
  while(anEdgeExplorer.More()){
      TopoDS_Edge anEdge = TopoDS::Edge(anEdgeExplorer.Current());
      //Add edge to fillet algorithm
      mkFillet.Add(myThickness / 12., anEdge);
      anEdgeExplorer.Next();
  }
 
  myBody = mkFillet.Shape();
 
  // Body : Add the Neck
  gp_Pnt neckLocation(0, 0, myHeight);
  gp_Dir neckAxis = gp::DZ();
  gp_Ax2 neckAx2(neckLocation, neckAxis);
 
  Standard_Real myNeckRadius = myThickness / 4.;
  Standard_Real myNeckHeight = myHeight / 10.;
 
  BRepPrimAPI_MakeCylinder MKCylinder(neckAx2, myNeckRadius, myNeckHeight);
  TopoDS_Shape myNeck = MKCylinder.Shape();
 
  myBody = BRepAlgoAPI_Fuse(myBody, myNeck);
 
  // Body : Create a Hollowed Solid
  TopoDS_Face   faceToRemove;
  Standard_Real zMax = -1;
 
  for(TopExp_Explorer aFaceExplorer(myBody, TopAbs_FACE); aFaceExplorer.More(); aFaceExplorer.Next()){
      TopoDS_Face aFace = TopoDS::Face(aFaceExplorer.Current());
      // Check if <aFace> is the top face of the bottle's neck 
      Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
      if(aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane)){
          Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
          gp_Pnt aPnt = aPlane->Location();
          Standard_Real aZ   = aPnt.Z();
          if(aZ > zMax){
              zMax = aZ;
              faceToRemove = aFace;
          }
      }
  }
 
  TopTools_ListOfShape facesToRemove;
  facesToRemove.Append(faceToRemove);
  BRepOffsetAPI_MakeThickSolid aSolidMaker;
  aSolidMaker.MakeThickSolidByJoin(myBody, facesToRemove, -myThickness / 50, 1.e-3);
  myBody = aSolidMaker.Shape();
  // Threading : Create Surfaces
  Handle(Geom_CylindricalSurface) aCyl1 = new Geom_CylindricalSurface(neckAx2, myNeckRadius * 0.99);
  Handle(Geom_CylindricalSurface) aCyl2 = new Geom_CylindricalSurface(neckAx2, myNeckRadius * 1.05);
 
  // Threading : Define 2D Curves
  gp_Pnt2d aPnt(2. * M_PI, myNeckHeight / 2.);
  gp_Dir2d aDir(2. * M_PI, myNeckHeight / 4.);
  gp_Ax2d anAx2d(aPnt, aDir);
 
  Standard_Real aMajor = 2. * M_PI;
  Standard_Real aMinor = myNeckHeight / 10;
 
  Handle(Geom2d_Ellipse) anEllipse1 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor);
  Handle(Geom2d_Ellipse) anEllipse2 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor / 4);
  Handle(Geom2d_TrimmedCurve) anArc1 = new Geom2d_TrimmedCurve(anEllipse1, 0, M_PI);
  Handle(Geom2d_TrimmedCurve) anArc2 = new Geom2d_TrimmedCurve(anEllipse2, 0, M_PI);
  gp_Pnt2d anEllipsePnt1 = anEllipse1->Value(0);
  gp_Pnt2d anEllipsePnt2 = anEllipse1->Value(M_PI);
 
  Handle(Geom2d_TrimmedCurve) aSegment = GCE2d_MakeSegment(anEllipsePnt1, anEllipsePnt2);
  // Threading : Build Edges and Wires
  TopoDS_Edge anEdge1OnSurf1 = BRepBuilderAPI_MakeEdge(anArc1, aCyl1);
  TopoDS_Edge anEdge2OnSurf1 = BRepBuilderAPI_MakeEdge(aSegment, aCyl1);
  TopoDS_Edge anEdge1OnSurf2 = BRepBuilderAPI_MakeEdge(anArc2, aCyl2);
  TopoDS_Edge anEdge2OnSurf2 = BRepBuilderAPI_MakeEdge(aSegment, aCyl2);
  TopoDS_Wire threadingWire1 = BRepBuilderAPI_MakeWire(anEdge1OnSurf1, anEdge2OnSurf1);
  TopoDS_Wire threadingWire2 = BRepBuilderAPI_MakeWire(anEdge1OnSurf2, anEdge2OnSurf2);
  BRepLib::BuildCurves3d(threadingWire1);
  BRepLib::BuildCurves3d(threadingWire2);
 
  // Create Threading 
  BRepOffsetAPI_ThruSections aTool(Standard_True);
  aTool.AddWire(threadingWire1);
  aTool.AddWire(threadingWire2);
  aTool.CheckCompatibility(Standard_False);
 
  TopoDS_Shape myThreading = aTool.Shape();
 
  // Building the Resulting Compound 
  TopoDS_Compound aRes;
  BRep_Builder aBuilder;
  aBuilder.MakeCompound (aRes);
  aBuilder.Add (aRes, myBody);
  aBuilder.Add (aRes, myThreading);

  interp->GetPlotter().REDRAW_SHAPE("bottle", aRes);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Test(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("test",        "Test anything.", __FILE__, group, MISC_Test);
  interp->AddCommand("test-bottle", "Test bottle.",   __FILE__, group, MISC_TestBottle);
}
