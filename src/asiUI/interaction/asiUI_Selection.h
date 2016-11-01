//-----------------------------------------------------------------------------
// Created on: 26 November 2015
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

#ifndef asiUI_Selection_h
#define asiUI_Selection_h

// UI includes
#include <asiUI.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <Standard_Type.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>

// VTK includes
#include <vtkActor.h>
#include <vtkSmartPointer.h>

// Qt includes
#pragma warning(push, 0)
#include <QtCore/QPoint>
#pragma warning(pop)

//-----------------------------------------------------------------------------

//! Picker type.
enum asiUI_PickType
{
  PickType_Cell = 0,
  PickType_Point,
  PickType_World
};

//-----------------------------------------------------------------------------

//! Selection type.
enum asiUI_SelectionNature
{
  SelectionNature_None,     //!< Nothing.
  SelectionNature_Pick = 1, //!< User makes final decision on selection.
  SelectionNature_Detection //!< User observes selection possibilities.
};

//-----------------------------------------------------------------------------

//! Selection mode.
enum asiUI_SelectionMode
{
  SelectionMode_None      = 0x0001, //!< Selection is disabled.
  SelectionMode_Workpiece = 0x0002, //!< Entire shape or mesh.
  SelectionMode_Face      = 0x0004, //!< Faces only.
  SelectionMode_Edge      = 0x0008, //!< Edges only.
  SelectionMode_Vertex    = 0x0010, //!< Vertices only.

  //! Group of NONE selection mode only.
  SelectionMode_DummySet  = SelectionMode_None,

  //! Group of NONE and WORKPIECE selection modes.
  SelectionMode_GlobalSet = SelectionMode_None |
                            SelectionMode_Workpiece,

  //! Group of local selection modes: FACE, EDGE, VERTEX.
  SelectionMode_LocalSet  = SelectionMode_Face |
                            SelectionMode_Edge |
                            SelectionMode_Vertex,

  //! Group of all possible selection modes.
  SelectionMode_Unverse   = SelectionMode_DummySet |
                            SelectionMode_GlobalSet |
                            SelectionMode_LocalSet
};

//-----------------------------------------------------------------------------

//! Class representing picking inputs. This structure is designed to be
//! used in pick callbacks in order to retrieve all data required to
//! shoot the actual picking request.
struct asiUI_PickInput
{
  QPoint Start;      //!< Picked X coordinate.
  QPoint Finish;     //!< Picked Y coordinate.
  bool   IsMultiple; //!< Indicates whether the multiple picking is enabled.

  //! Default constructor
  asiUI_PickInput()
  {
    this->IsMultiple = false;
  }

  //! Complete constructor.
  //! \param theStart   [in] start picking point.
  //! \param theFinish  [in] finish picking point.
  //! \param isMultiple [in] indicates whether the multiple picking is enabled.
  asiUI_PickInput(const QPoint& theStart,
                  const QPoint& theFinish,
                  const bool    isMultiple)
  {
    this->Start      = theStart;
    this->Finish     = theFinish;
    this->IsMultiple = isMultiple;
  }
};

//-----------------------------------------------------------------------------

//! Type short-cut for correspondence map between VTK actors and their
//! sub-element masks.
typedef NCollection_DataMap<vtkSmartPointer<vtkActor>,
                            TColStd_PackedMapOfInteger> asiVisu_ActorElemMap;

//-----------------------------------------------------------------------------

//! Class representing picking results for different kinds of selection.
class asiUI_PickResult
{
public:

  asiUI_PickResult(const int theSelModes = SelectionMode_None);
  ~asiUI_PickResult();

public:

  asiUI_PickResult& operator<<(const vtkSmartPointer<vtkActor>& theActor);
  asiUI_PickResult& operator<<(const vtkIdType theElemID);
  asiUI_PickResult& operator<<(const TColStd_PackedMapOfInteger& theElemMask);

public:

  void SetSelectionModes(const int theSelModes);

public:

  const asiVisu_ActorElemMap& GetPickMap           ()                  const;
  int                         NbElements           ()                  const;
  void                        Clear                ();
  bool                        IsEmpty              ()                  const;
  bool                        DoesSelectionCover   (const int theMode) const;
  bool                        IsSelectionEqual     (const int theMode) const;
  bool                        IsSelectionNone      ()                  const;
  bool                        IsSelectionWorkpiece ()                  const;
  bool                        IsSelectionFace      ()                  const;
  bool                        IsSelectionEdge      ()                  const;
  bool                        IsSelectionVertex    ()                  const;
  bool                        IsSelectionSubShape  ()                  const;

private:

  //! Selection modes the results were obtained for.
  int m_iSelModes;

  //! Internal collection for picking results.
  asiVisu_ActorElemMap m_pickMap;

  //! Previously streamed actor.
  vtkSmartPointer<vtkActor> m_prevActor;

};

#endif
