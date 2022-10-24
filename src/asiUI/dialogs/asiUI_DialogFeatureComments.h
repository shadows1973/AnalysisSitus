//-----------------------------------------------------------------------------
// Created on: 20 October 2022
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

#ifndef asiUI_DialogFeatureComments_h
#define asiUI_DialogFeatureComments_h

// asiUI includes
#include <asiUI_StyledTextEdit.h>

// asiData includes
#include <asiData_FeatureNode.h>

// Active Data includes
#include <ActAPI_IModel.h>
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#pragma warning(pop)

//-----------------------------------------------------------------------------

//! Dialog for exposing comments stored in Feature Nodes.
class asiUI_DialogFeatureComments : public QDialog
{
  Q_OBJECT

public:

  asiUI_EXPORT
    asiUI_DialogFeatureComments(const Handle(ActAPI_IModel)&       model,
                                const Handle(asiData_FeatureNode)& node,
                                ActAPI_ProgressEntry               progress,
                                ActAPI_PlotterEntry                plotter,
                                QWidget*                           parent = nullptr);

  asiUI_EXPORT virtual
    ~asiUI_DialogFeatureComments();

public:

  asiUI_EXPORT virtual void
    Initialize();

public slots:

  void onApply();
  void onClose();

protected:

  QVBoxLayout* m_pMainLayout; //!< Layout of the widget.

  //! Widgets.
  struct t_base_widgets
  {
    QTextEdit*    pText;  //!< Text editor.
    QPushButton*  pApply; //!< Apply.
    QPushButton*  pClose; //!< Close.

    t_base_widgets() : pText  (nullptr),
                       pApply (nullptr),
                       pClose (nullptr)
    {}

    void Release()
    {
      delete pText;  pText  = nullptr;
      delete pApply; pApply = nullptr;
      delete pClose; pClose = nullptr;
    }
  };

  t_base_widgets m_widgets; //!< Involved widgets.

protected:

  Handle(ActAPI_IModel)       m_model;    //!< Data Model instance.
  Handle(asiData_FeatureNode) m_node;     //!< Data Node.
  ActAPI_ProgressEntry        m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry         m_plotter;  //!< Imperative plotter.

};

#endif
