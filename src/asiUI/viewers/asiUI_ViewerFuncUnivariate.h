//-----------------------------------------------------------------------------
// Created on: 04 February 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiUI_ViewerFuncUnivariate_h
#define asiUI_ViewerFuncUnivariate_h

// A-Situs includes
#include <asiUI.h>

// A-Situs (GUI) includes
#include <asiUI_Viewer.h>

// A-Situs (visualization) includes
#include <asiVisu_InteractorStylePick.h>
#include <asiUI_PickCallback.h>

// VTK includes
#include <vtkTextWidget.h>

//! Viewer for a univariate function in explicit form.
class asiUI_ViewerFuncUnivariate : public asiUI_Viewer
{
  Q_OBJECT

public:

  asiUI_EXPORT
    asiUI_ViewerFuncUnivariate(QWidget* parent = NULL);

  asiUI_EXPORT virtual
    ~asiUI_ViewerFuncUnivariate();

public:

  void Repaint();

public slots:

  void onResetView();

private:

  vtkSmartPointer<vtkTextWidget> m_textWidget; //!< Annotation.

};

#endif
