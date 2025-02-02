//-----------------------------------------------------------------------------
// Created on: 21 February 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Anton Poletaev, Sergey Slyadnev
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

// Own include
#include <asiUI_DatumColorItem.h>

// asiUI includes
#include <asiUI_Common.h>

// Qt includes
#pragma warning(push, 0)
#include <QHBoxLayout>
#include <QLineEdit>
#pragma warning(pop)

//! Constructor.
//! \param theDicID [in] id of the dictionary item for datum.
//! \param theParent [in] parent widget for subwidget controls.
//! \param theDatumFlags [in] datum subwidget and option flags.
asiUI_DatumColorItem::asiUI_DatumColorItem(const QString& theDicID,
                                           QWidget*       theParent,
                                           const int      theDatumFlags)
: asiUI_Datum(theParent)
{
  m_pEditor = new Editor(theDicID, theParent, convertFlags(theDatumFlags));

  init(m_pEditor);

  connect(m_pEditor, SIGNAL(ColorChanged(const int)), 
          this,      SIGNAL(ColorChanged(const int)));
}

//! Update unit system.
void asiUI_DatumColorItem::onUnitsUpdated()
{
  m_pEditor->UpdateUnits();
}

//! Get datum.
//! \return suit datum pointer.
QDS_Datum* asiUI_DatumColorItem::getDatum() const
{
  return m_pEditor;
}

//-----------------------------------------------------------------------------
// Editor control
//-----------------------------------------------------------------------------

//! Constructor. Initializes controls.
asiUI_DatumColorItem::Editor::Editor(const QString& theDicID,
                                     QWidget* theParent,
                                     const int theFlags)
: QDS_Datum(theDicID, theParent, theFlags)
{
}

//! Create control widget
//! \param theParent [in] parent widget for editor.
//! \return widget pointer.
QWidget* asiUI_DatumColorItem::Editor::createControl(QWidget* theParent)
{
  asiUI_DatumColorItem::ColorItemDialog* anEditor =
    new asiUI_DatumColorItem::ColorItemDialog(theParent);

  connect(anEditor, SIGNAL(ColorChanged(const QColor&)),
          this,     SLOT(onColorChanged(const QColor&)));

  return anEditor;
}

//! Get color dialog control.
//! \return pointer to a color dialog control.
QColorDialog* asiUI_DatumColorItem::Editor::colorDialog() const
{
  return ::qobject_cast<QColorDialog*>( controlWidget() );
}

