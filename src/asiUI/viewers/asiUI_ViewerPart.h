//-----------------------------------------------------------------------------
// Created on: 27 November 2015
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiUI_ViewerPart_h
#define asiUI_ViewerPart_h

// A-Situs includes
#include <asiUI.h>

// Common includes
#include <asiEngine_Model.h>

// GUI includes
#include <asiUI_PartCallback.h>
#include <asiUI_PickCallback.h>
#include <asiUI_RotationCallback.h>
#include <asiUI_Viewer.h>

// Visualization includes
#include <asiVisu_InteractorStylePick.h>

// VTK includes
#include <vtkOrientationMarkerWidget.h>

//! Viewer for part.
class asiUI_ViewerPart : public asiUI_Viewer
{
  Q_OBJECT

public:

  asiUI_EXPORT
    asiUI_ViewerPart(const Handle(asiEngine_Model)& model,
                     QWidget*                       parent = NULL);

  asiUI_EXPORT virtual
    ~asiUI_ViewerPart();

public:

  asiUI_EXPORT void
    Repaint();

public:

  //! \return pick callback.
  const vtkSmartPointer<asiUI_PickCallback>& GetPickCallback() const
  {
    return m_pickCallback;
  }

public slots:

  void onResetView();
  void onSubShapesPicked();
  void onFindFace();
  void onFindEdge();
  void onRefineTessellation();
  void onContextMenu(const QPoint&);

signals:

  void facePicked(const asiVisu_PickResult&);
  void edgePicked(const asiVisu_PickResult&);
  void contextMenu(const QPoint&);

protected:

  //! Data Model instance.
  Handle(asiEngine_Model) m_model;

  //! Part callback.
  vtkSmartPointer<asiUI_PartCallback> m_partCallback;

  //! Pick callback.
  vtkSmartPointer<asiUI_PickCallback> m_pickCallback;

  //! Rotation callback.
  vtkSmartPointer<asiUI_RotationCallback> m_rotoCallback;

  //! Orientation marker widget.
  vtkSmartPointer<vtkOrientationMarkerWidget> m_axesWidget;

};

#endif
