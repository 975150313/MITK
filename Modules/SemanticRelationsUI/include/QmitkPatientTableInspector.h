/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKPATIENTTABLEINSPECTOR_H
#define QMITKPATIENTTABLEINSPECTOR_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <QmitkAbstractSemanticRelationsStorageInspector.h>
#include <QmitkPatientTableModel.h>

#include "ui_QmitkPatientTableInspector.h"

// qt widgets module
#include "QmitkEnums.h"

// qt
#include <QMenu>

/*
* @brief The QmitkPatientTableInspector is a QmitkAbstractSemanticRelationsStorageInspector that shows the currently
*        available data of the semantic relations storage model in a control-point - information type matrix.
*
*   The QmitkPatientTableInspector uses the QmitkSemanticRelationsStorageModel, a QmitkAbstractDataStorageModel that
*   presents the semantic relations data as a table, showing a QPixmap as thumbnail for the data nodes.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientTableInspector : public QmitkAbstractSemanticRelationsStorageInspector
{
  Q_OBJECT

public:

  QmitkPatientTableInspector(QWidget* parent = nullptr);

  virtual QAbstractItemView* GetView() override;
  virtual const QAbstractItemView* GetView() const override;

  virtual void SetSelectionMode(SelectionMode mode) override;
  virtual SelectionMode GetSelectionMode() const override;

  virtual void SetCaseID(const mitk::SemanticTypes::CaseID& caseID) override;
  virtual void SetLesion(const mitk::SemanticTypes::Lesion& lesion) override;

Q_SIGNALS:

  void DataNodeDoubleClicked(const mitk::DataNode::Pointer);

private Q_SLOTS:

  void OnModelUpdated();
  void OnNodeButtonClicked(const QString&);
  void OnTableViewContextMenuRequested(const QPoint&);
  void OnContextMenuSetInformationType();
  void OnContextMenuSetControlPoint();
  void OnItemDoubleClicked(const QModelIndex&);

protected:

  virtual void Initialize() override;

private:

  void SetUpConnections();

  Ui::QmitkPatientTableInspector m_Controls;
  QmitkPatientTableModel* m_StorageModel;

  QMenu* m_ContextMenu;
  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKPATIENTTABLEINSPECTOR_H
