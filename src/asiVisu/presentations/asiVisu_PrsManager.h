//-----------------------------------------------------------------------------
// Created on: 25 November 2015
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiVisu_PrsManager_h
#define asiVisu_PrsManager_h

// Visualization includes
#include <asiVisu_InteractorStylePick.h>
#include <asiVisu_InteractorStylePick2d.h>
#include <asiVisu_Prs.h>

// Active Data (API) includes
#include <ActAPI_IModel.h>

// VTK includes
#include <vtkAxesActor.h>
#include <vtkButtonWidget.h>
#include <vtkCellPicker.h>
#include <vtkCubeAxesActor.h>
#include <vtkObject.h>
#include <vtkPointPicker.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkWorldPointPicker.h>

// QVTK includes
#pragma warning(push, 0)
#include <QVTKWidget.h>
#pragma warning(pop)

// VIS includes
#pragma warning(push, 0)
#include <IVtkTools_ShapePicker.hxx>
#pragma warning(pop)

// OCCT includes
#include <NCollection_DataMap.hxx>

class asiVisu_AxesBtnCallback;

//! This class is designed to handle 3D representations. Besides that,
//! presentation manager controls QVTK widget along with all standard
//! visualization suite like Renderer, Render Window, etc.
//!
//! presentation manager is a facade under visualization. It connects
//! Data Model with viewer. Moreover, it manages interactive detection
//! and picking.
//!
//! This class inherits vtkObject in order to take advantage of VTK standard
//! observer mechanism.
class asiVisu_PrsManager : public vtkObject
{
public: // VTK macros and methods to override

  vtkTypeMacro(asiVisu_PrsManager, vtkObject);

  asiVisu_EXPORT static asiVisu_PrsManager*
    New();

  asiVisu_EXPORT void
    PrintSelf(ostream& os, vtkIndent indent);

public:

  //! Interaction dimensionality.
  enum InteractionMode
  {
    InteractionMode_3D,
    InteractionMode_2D
  };

public:

  asiVisu_EXPORT static void
    PlaceButton(vtkButtonWidget* pButton,
                vtkRenderer*     pRenderer);

  asiVisu_EXPORT static void
    CreateImage(vtkSmartPointer<vtkImageData> image,
                unsigned char*                color1,
                unsigned char*                color2);

public:

  asiVisu_EXPORT
    asiVisu_PrsManager();

public:

  asiVisu_EXPORT void
    SetBlackAndWhiteIntensity(const double black,
                              const double white);

// Presentation management:
public:

  asiVisu_EXPORT virtual void
    Actualize(const Handle(ActAPI_INode)& node,
              const bool                  withChildren  = false,
              const bool                  doFitContents = false,
              const bool                  withRepaint   = true);

  asiVisu_EXPORT virtual void
    Actualize(const Handle(ActAPI_HNodeList)& nodeList,
              const bool                      withChildren  = false,
              const bool                      doFitContents = false,
              const bool                      withRepaint   = true);

  asiVisu_EXPORT virtual void
    ActualizeExclusively(const Handle(ActAPI_HNodeList)& nodeList,
                         const bool                      doFitContents = true);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual bool
    IsPresented(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT virtual bool
    IsPresented(const ActAPI_DataObjectId& nodeId);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual bool
    SetPresentation(const Handle(ActAPI_INode)& node);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual Handle(asiVisu_Prs)
    GetPresentation(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT virtual Handle(asiVisu_Prs)
    GetPresentation(const ActAPI_DataObjectId& nodeId);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual void
    InitPresentation(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT virtual void
    InitPresentation(const ActAPI_DataObjectId& nodeId);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual void
    RenderPresentation(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT virtual void
    RenderPresentation(const ActAPI_DataObjectId& nodeId);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual void
    DeRenderPresentation(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT virtual void
    DeRenderPresentation(const ActAPI_DataObjectId& nodeId);

  asiVisu_EXPORT virtual void
    DeRenderAllPresentations();

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual void
    UpdatePresentation(const Handle(ActAPI_INode)& node,
                       const bool doFitContents = true);

  asiVisu_EXPORT virtual void
    UpdatePresentation(const ActAPI_DataObjectId& nodeId,
                       const bool doFitContents = true);

//-----------------------------------------------------------------------------

  asiVisu_EXPORT virtual bool
    DeletePresentation(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT virtual bool
    DeletePresentation(const ActAPI_DataObjectId& nodeId);

  asiVisu_EXPORT virtual void
    DeleteAllPresentations();

//-----------------------------------------------------------------------------

// Selection management:
public:

  asiVisu_EXPORT void
    SetSelectionMode(const int node);

  asiVisu_EXPORT int
    GetSelectionMode() const;

  asiVisu_EXPORT ActAPI_DataObjectIdList
    Pick(asiVisu_PickInput*            pickInput,
         const asiVisu_SelectionNature selNature,
         const asiVisu_PickType        pickType = PickType_Cell);

  asiVisu_EXPORT void
    SetPickList(const Handle(ActAPI_HNodeList)& nodeList);

  asiVisu_EXPORT const Handle(ActAPI_HNodeList)&
    GetPickList() const;

  asiVisu_EXPORT void
    SetPickFromList(const bool isEnabled) const;

  asiVisu_EXPORT bool
    IsPickFromList() const;

  asiVisu_EXPORT void
    Highlight(const Handle(ActAPI_HNodeList)& nodeList);

  asiVisu_EXPORT void
    Highlight(const Handle(ActAPI_INode)& node);

  asiVisu_EXPORT void
    Highlight(const Handle(ActAPI_HNodeList)& nodeList,
              const asiVisu_ActorElemMap&     actorElems,
              const int                       modes);

  asiVisu_EXPORT void
    CleanDetection();

  asiVisu_EXPORT Handle(ActAPI_HNodeList)
    GetHighlighted() const;

  asiVisu_EXPORT const asiVisu_ActualSelection&
    GetCurrentSelection() const;

// Visualization kernel:
public:

  asiVisu_EXPORT long int
    AddUpdateCallback(unsigned long eventID,
                      vtkCommand*   pCallback);

  asiVisu_EXPORT bool
    RemoveUpdateCallback(unsigned long eventID,
                         unsigned long tag);

  asiVisu_EXPORT void
    SetRenderer(const vtkSmartPointer<vtkRenderer>& renderer);

  asiVisu_EXPORT vtkRenderer*
    GetRenderer() const;

  asiVisu_EXPORT vtkRenderWindow*
    GetRenderWindow() const;

  asiVisu_EXPORT void
    Initialize(QWidget*   pWidget,
               const bool isOffscreen = false);

  asiVisu_EXPORT void
    InitializePickers();

  asiVisu_EXPORT QVTKWidget*
    GetQVTKWidget() const;

  asiVisu_EXPORT vtkInteractorStyle*
    GetDefaultInteractorStyle() const;

  asiVisu_EXPORT vtkInteractorStyle*
    GetImageInteractorStyle() const;

  asiVisu_EXPORT vtkAxesActor*
    GetTrihedron() const;

  asiVisu_EXPORT vtkSmartPointer<vtkPropCollection>
    PropsByTrihedron() const;

  asiVisu_EXPORT const vtkSmartPointer<vtkCellPicker>&
    GetCellPicker() const;

  asiVisu_EXPORT const vtkSmartPointer<IVtkTools_ShapePicker>&
    GetShapePicker() const;

public:

  void SetInteractionMode(const InteractionMode mode)
  {
    if ( mode == InteractionMode_3D )
      m_renderWindowInteractor->SetInteractorStyle(m_interactorStyleTrackball);
    if ( mode == InteractionMode_2D )
      m_renderWindowInteractor->SetInteractorStyle(m_interactorStyleImage);
  }

// Auxiliary methods:
protected:

  asiVisu_EXPORT void
    adjustTrihedron();

  asiVisu_EXPORT void
    actualizeShapeSelectionMode();

public:

  double BlackIntensity;
  double WhiteIntensity;
  bool   isWhiteBackground;

// Presentation management:
private:

  //! Shortcut for mapping type between Nodes and Presentations. We use
  //! Node ID as key type in order not to deal with data objects when
  //! storing/retrieving Node Information to/from pipelines.
  typedef NCollection_DataMap<ActAPI_DataObjectId, Handle(asiVisu_Prs)> TNodePrsMap;

  //! Mapping between Nodes and their Presentations.
  TNodePrsMap m_nodePresentations;

// Selection management:
private:

  //! Picker to select VTK topology (cells).
  vtkSmartPointer<vtkCellPicker> m_cellPicker;

  //! Picker to select VTK geometry (points).
  vtkSmartPointer<vtkPointPicker> m_pointPicker;

  //! Picker to select any point (not necessarily from point data).
  vtkSmartPointer<vtkWorldPointPicker> m_worldPicker;

  //! Picker to select sub-shapes.
  vtkSmartPointer<IVtkTools_ShapePicker> m_shapePicker;

  //! Currently selected Presentations.
  asiVisu_ActualSelection m_currentSelection;

// Visualization kernel:
private:

  //! QVTK widget.
  QVTKWidget* m_widget;

  //! Active renderer.
  vtkSmartPointer<vtkRenderer> m_renderer;

  //! Render Window.
  vtkSmartPointer<vtkRenderWindow> m_renderWindow;

  //! Render Window Interactor.
  vtkSmartPointer<vtkRenderWindowInteractor> m_renderWindowInteractor;

  //! Default Interactor Style.
  vtkSmartPointer<asiVisu_InteractorStylePick> m_interactorStyleTrackball;

  //! Interactor Style for 2D scenes.
  vtkSmartPointer<asiVisu_InteractorStylePick2d> m_interactorStyleImage;

  //! Axes actor.
  vtkSmartPointer<vtkAxesActor> m_trihedron;

  //! Button to toggle axes.
  vtkSmartPointer<vtkButtonWidget> m_axesButton;

  //! Callback for axes button.
  vtkSmartPointer<asiVisu_AxesBtnCallback> m_axesCallback;

  //! List of nodes allowed for picking
  Handle(ActAPI_HNodeList) m_bAllowedNodes;

  //! Selection modes.
  int m_iSelectionModes;

  //! Custom Update Presentation callbacks.
  //! They are called in UpdatePresentation() method and
  //! provide possibility to perform custom actions when
  //! presentation manager is updating some of its presentations.
  NCollection_Sequence<unsigned long> m_updateCallbackIds;

};

#endif
