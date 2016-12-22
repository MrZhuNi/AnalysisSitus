//-----------------------------------------------------------------------------
// Created on: 15 April 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiAlgo_CommonCloud2d_h
#define asiAlgo_CommonCloud2d_h

// A-Situs includes
#include <asiAlgo.h>

// OCCT includes
#include <gp_XY.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColgp_HSequenceOfPnt2d.hxx>
#include <TColStd_HArray1OfReal.hxx>

// STL includes
#include <vector>

//-----------------------------------------------------------------------------
// Single point with additional information
//-----------------------------------------------------------------------------

//! Point enriched with additional information used in algorithms.
template <typename TCoord>
struct asiAlgo_CommonCloud
{
  //! Copy ctor.
  //! \param P [in] point to initialize this one from.
  asiAlgo_CommonCloud(const asiAlgo_CommonCloud& P) : Coord(P.Coord), Status(P.Status), Index(P.Index), pData(P.pData) {}

  //! Complete ctor.
  //! \param _coord    [in] spatial coordinates.
  //! \param _status   [in] status (deleted, not deleted).
  //! \param _index    [in] index of the point in external collection.
  //! \param _data_ptr [in] user data.
  asiAlgo_CommonCloud(const TCoord _coord    = TCoord(),
                      const int    _status   = 0,
                      const size_t _index    = 0,
                      void*        _data_ptr = NULL) : Coord(_coord), Status(_status), Index(_index), pData(_data_ptr) {}

  bool operator>  (const asiAlgo_CommonCloud&) const { return false; }
  bool operator== (const asiAlgo_CommonCloud&) const { return false; }

  TCoord Coord;  //!< Coordinates.
  int    Status; //!< General-purpose status.
  size_t Index;  //!< Index in the owning collection.
  void*  pData;  //!< Associated data.
};

//! Collection of 2D points with statuses.
typedef std::vector< asiAlgo_CommonCloud<gp_XY>, std::allocator< asiAlgo_CommonCloud<gp_XY> > > asiAlgo_CommonCloud2dVec;

//-----------------------------------------------------------------------------
// Point cloud
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(asiAlgo_CommonCloud2d, Standard_Transient)

//! Data adapter for two-dimensional point cloud.
class asiAlgo_CommonCloud2d : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_CommonCloud2d, Standard_Transient)

public:

  asiAlgo_EXPORT static Handle(asiAlgo_CommonCloud2d)
    RemoveCoincidentPoints(const Handle(asiAlgo_CommonCloud2d)& pts,
                           const double                         prec);

public:

  asiAlgo_EXPORT virtual Handle(asiAlgo_CommonCloud2d)
    EmptyCopy() const = 0;

  asiAlgo_EXPORT virtual const gp_XY&
    Value(const int oneBasedIndex) const = 0;

  asiAlgo_EXPORT virtual const int
    Length() const = 0;

  asiAlgo_EXPORT virtual Handle(TColStd_HArray1OfReal)
    Coords2d() const = 0;

  asiAlgo_EXPORT virtual void
    SetCoords2d(const Handle(TColStd_HArray1OfReal)& coords) = 0;

  asiAlgo_EXPORT virtual void
    AddCoord(const gp_XY& coord) = 0;

public:

  //! Reads point cloud from file.
  //! \param filename [in]  filename of the target file.
  //! \param points   [out] retrieved points.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    Read(const TCollection_AsciiString&   filename,
         Handle(TColgp_HSequenceOfPnt2d)& points);

  //! Writes point cloud to file.
  //! \param points   [in] points to write.
  //! \param filename [in] filename of the target file.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    Write(const Handle(TColgp_HSequenceOfPnt2d)& points,
          const TCollection_AsciiString&         filename);

};

#endif
