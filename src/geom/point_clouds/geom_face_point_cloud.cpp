//-----------------------------------------------------------------------------
// Created on: 16 April 2016
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <geom_face_point_cloud.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>

//! Default constructor.
geom_face_point_cloud::geom_face_point_cloud()
{}

//! Initializes point cloud with vertices from the given face.
//! \param face [in] base face.
void geom_face_point_cloud::InitWithVertices(const TopoDS_Face& face)
{
  m_face = face;

  // Populate point cloud
  for ( TopExp_Explorer exp(m_face, TopAbs_VERTEX); exp.More(); exp.Next() )
  {
    const TopoDS_Vertex& V = TopoDS::Vertex( exp.Current() );
    gp_Pnt               P = BRep_Tool::Pnt(V);
    //
    m_points2d.push_back( BRep_Tool::Parameters(V, m_face).XY() );
    m_points3d.push_back( P.XYZ() );
  }
}

//! Returns coordinates by 1-based index.
//! \param oneBasedIndex [in] 1-based index of a point.
//! \return coordinates.
const gp_XYZ& geom_face_point_cloud::Value3d(const int oneBasedIndex) const
{
  return m_points3d[oneBasedIndex - 1];
}

//! \return base face.
const TopoDS_Face& geom_face_point_cloud::Face() const
{
  return m_face;
}

//! Sets base face.
//! \param face [in] face to set.
void geom_face_point_cloud::SetFace(const TopoDS_Face& face)
{
  m_face = face;
}

//! Creates empty copy of the point cloud.
//! \return empty copy.
Handle(geom_common_point_cloud_2d) geom_face_point_cloud::EmptyCopy() const
{
  Handle(geom_face_point_cloud) cloud = new geom_face_point_cloud;
  cloud->SetFace(m_face);
  return cloud;
}

//! Returns coordinates by 1-based index.
//! \param oneBasedIndex [in] 1-based index of a point.
//! \return coordinates.
const gp_XY& geom_face_point_cloud::Value(const int oneBasedIndex) const
{
  return m_points2d[oneBasedIndex - 1];
}

//! \return number of points.
const int geom_face_point_cloud::Length() const
{
  return (int) m_points2d.size();
}

//! \return coordinates of the managed points as a plain array.
Handle(TColStd_HArray1OfReal) geom_face_point_cloud::Coords2d() const
{
  const int numPts = this->Length();
  if ( !numPts )
    return NULL;

  Handle(TColStd_HArray1OfReal) result = new TColStd_HArray1OfReal(0, numPts - 1);
  //
  for ( int pidx = 0; pidx < numPts; ++pidx )
  {
    result->ChangeValue( pidx*2 )     = this->Value(pidx + 1).X();
    result->ChangeValue( pidx*2 + 1 ) = this->Value(pidx + 1).Y();
  }

  return result;
}

//! Initializes point cloud from the given array.
//! \param coords [in] coordinates of the points.
void geom_face_point_cloud::SetCoords2d(const Handle(TColStd_HArray1OfReal)& coords)
{
  m_points2d.clear();
  m_points3d.clear();
  //
  if ( coords.IsNull() )
    return;

  for ( int coordidx = 0; coordidx < coords->Length(); coordidx += 2 )
  {
    gp_XY pt( coords->Value(coordidx), coords->Value(coordidx + 1) );
    m_points2d.push_back(pt);
  }
}

//! Adds the passed point to the point cloud.
//! \param coord [in] point to add.
void geom_face_point_cloud::AddCoord(const gp_XY& coord)
{
  // Add 2D point
  m_points2d.push_back(coord);

  // Restore 3D
  Handle(Geom_Surface) S = BRep_Tool::Surface(m_face);
  gp_Pnt p3d = S->Value( coord.X(), coord.Y() );

  // Add 3D point
  m_points3d.push_back( p3d.XYZ() );
}
