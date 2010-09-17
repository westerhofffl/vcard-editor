#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Project.h"
#include "ProjectSettingsDialog.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QImage>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_project(0)
{
    m_ui->setupUi(this);

    connect(m_ui->actionNew, SIGNAL(triggered()), SLOT(createNewProject()));
    connect(m_ui->pauseButton, SIGNAL(clicked()), SLOT(startStopProject()));

    connect(m_ui->filterComboBox, SIGNAL(currentIndexChanged(int)),
       SLOT(updateFilter()));

    connect(m_ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(showTreePreview(QTreeWidgetItem*)));

    connect(m_ui->treePreviewButton, SIGNAL(clicked()), SLOT(showExternTreePreview()));

    connect(m_ui->tableWidget, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
       SLOT(showTablePreview(QTableWidgetItem*)));

    connect(m_ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
            SLOT(checkTableItemState(QTableWidgetItem*)));

    connect(m_ui->tablePreviewButton, SIGNAL(clicked()), SLOT(showExternTablePreview()));

    connect(m_ui->moveButton, SIGNAL(clicked()), SLOT(move()));
    connect(m_ui->unmoveButton, SIGNAL(clicked()), SLOT(unmove()));
    connect(m_ui->toggleMoveButton, SIGNAL(clicked()), SLOT(toggleMove()));

    showTreePreview(NULL);
    showTablePreview(NULL);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::createNewProject()
{
   ProjectSettingsDialog* settingsDialog = new ProjectSettingsDialog(this);
   if (settingsDialog->exec() == QDialog::Accepted)
   {
      delete m_project;
      m_project = new Project(settingsDialog->getFolderName(),
                              settingsDialog->getDuplicatesFolderName());
      connect(m_project, SIGNAL(progressStatus(int, QString)),
              SLOT(setProgressBar(int, QString)));
      connect(m_project, SIGNAL(groupUpdated(int)),
              SLOT(updateGroup(int)));
      startStopProject();
   }
}

void MainWindow::startStopProject()
{
   switch(m_project->getStatus())
   {
   case Project::NOT_STARTED:
      m_ui->folderLabel->setText(m_project->getFolderName());
      m_ui->duplicatesFolderLabel->setText(m_project->getDuplicatesFolderName());
      m_project->scan();
      m_ui->pauseButton->setText("Pause");
      break;
   case Project::SCANNING:
      m_project->pause();
      m_ui->pauseButton->setText("Continue");
      break;
   case Project::PAUSED:
      m_project->scan();
      m_ui->pauseButton->setText("Pause");
      break;
   case Project::FINISHED:
      m_project->rescan();
      m_ui->pauseButton->setText("Rescan");
      break;
   default:
      break;
   }
}

void MainWindow::setProgressBar(int progress, QString text)
{
   if (progress == -1)
   {
      m_ui->progressBar->setRange(0, 0);
      m_ui->progressBar->setFormat(text);
   }
   else
   {
      m_ui->progressBar->setRange(0, 100);
      m_ui->progressBar->setValue(progress);
      m_ui->progressBar->setFormat(QString("%1 (%p%)").arg(text));
   }
}


void MainWindow::updateGroup(int index)
{
   if (m_ui->treeWidget->topLevelItemCount() <= index)
   {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      m_ui->treeWidget->addTopLevelItem(item);
   }
   QTreeWidgetItem* groupItem = m_ui->treeWidget->topLevelItem(index);
   QList<int> fileIndexList = m_project->getGroupFileIndexSet(index).toList();
   qSort(fileIndexList);
   QSet<QString> fileNameSet;
   foreach(int fileIndex, fileIndexList)
   {
      fileNameSet.insert(m_project->getFileName(fileIndex));
   }
   QStringList fileNameList = fileNameSet.toList();
   qSort(fileNameList);
   groupItem->setText(0, fileNameList.join(","));
   int firstFileIndex = fileIndexList.first();
   groupItem->setData(0, TREE_ROLE_FILE_INDEX, firstFileIndex);
   groupItem->setData(0, TREE_ROLE_GROUP_INDEX, index);
   groupItem->setData(0, TREE_ROLE_FILE_COUNT, fileIndexList.size());
   groupItem->setText(2, QString::number(m_project->getFileSize(firstFileIndex)));
   QByteArray md4 = m_project->getFileMd4(firstFileIndex);
   groupItem->setText(3, m_project->getFileMd4(firstFileIndex).toHex());

   qDeleteAll(groupItem->takeChildren());
   int movedCount = 0;
   foreach(int fileIndex, fileIndexList)
   {
      QTreeWidgetItem* fileItem = new QTreeWidgetItem(groupItem);
      QFileInfo fileInfo(m_project->getFileFolderName(fileIndex), m_project->getFileName(fileIndex));
      fileItem->setText(0, fileInfo.absoluteFilePath());
      fileItem->setData(0, TREE_ROLE_FILE_INDEX, fileIndex);
      if (m_project->isFileMoved(fileIndex))
      {
         ++movedCount;
         fileItem->setText(1, "moved");
      }
   }
   groupItem->setText(1, QString("%1 (%2 left, %3 moved)")
                      .arg(fileIndexList.size())
                      .arg(fileIndexList.size() - movedCount)
                      .arg(movedCount));
   groupItem->setData(0, TREE_ROLE_MOVED_COUNT, movedCount);

   if (groupItem->isSelected())
   {
      updateTable(index, -1);
   }
   updateFilter(groupItem);
}


void MainWindow::updateFilter()
{
   for (int row = 0; row < m_ui->treeWidget->topLevelItemCount(); ++row)
   {
      QTreeWidgetItem* item = m_ui->treeWidget->topLevelItem(row);
      updateFilter(item);
   }
}

void MainWindow::updateFilter(QTreeWidgetItem* item)
{
   int fileCount = item->data(0, TREE_ROLE_FILE_COUNT).toInt();
   int movedCount = item->data(0, TREE_ROLE_MOVED_COUNT).toInt();
   switch(m_ui->filterComboBox->currentIndex())
   {
   case 0:
      item->setHidden(false);
      break;
   case 1:
      item->setHidden(fileCount <= 1);
      break;
   case 2:
      item->setHidden((fileCount <= 1) || ((fileCount - movedCount) <= 1));
      break;
   case 3:
      item->setHidden(fileCount != movedCount);
      break;
   default:
      break;
   };      
}
void MainWindow::showTreePreview(QTreeWidgetItem* item)
{
   int fileIndex = -1;
   if (item != NULL)
   {
      fileIndex = item->data(0, TREE_ROLE_FILE_INDEX).toInt();
   }
   showPreview(fileIndex, m_ui->treePreviewButton);

   if (item != NULL)
   {
      QVariant groupIndexVariant = item->data(0, TREE_ROLE_GROUP_INDEX);
      if (!groupIndexVariant.isValid())
      {
         groupIndexVariant = item->parent()->data(0, TREE_ROLE_GROUP_INDEX);
      }
      else
      {
         fileIndex = -1;
      }
      int groupIndex = groupIndexVariant.toInt();
      updateTable(groupIndex, fileIndex);
   }
}

void MainWindow::showExternTreePreview()
{
   QTreeWidgetItem* item = m_ui->treeWidget->currentItem();
   if (!item)
   {
      return;
   }
   int fileIndex = item->data(0, TREE_ROLE_FILE_INDEX).toInt();
   QString absoluteFilePath = m_project->getAbsoluteFilePath(fileIndex);
   absoluteFilePath.prepend("file://");
   QDesktopServices::openUrl(QUrl(absoluteFilePath));
}

void MainWindow::updateTable(int groupIndex, int fileIndex)
{
   QApplication::setOverrideCursor(Qt::WaitCursor);

   m_ui->tableWidget->clear();
   m_ui->tableWidget->setRowCount(0);
   m_ui->tableWidget->setColumnCount(0);
   QCoreApplication::processEvents();

   m_ui->tableWidget->setRowCount(100);
   m_ui->tableWidget->setColumnCount(10);

   QList<int> selectedFileList;
   if (fileIndex != -1)
   {
      selectedFileList.append(fileIndex);
   }
   else
   {
      selectedFileList = m_project->getGroupFileIndexSet(groupIndex).toList();
      qSort(selectedFileList);
   }

   QStringList columnList;
   QList<int> groupIndexList;
   QList<int> folderFileList = m_project->getFolderFileList(selectedFileList);
   groupIndexList = m_project->getGroupList(folderFileList);

   int row = 0;
   foreach(int groupIndex, groupIndexList)
   {
      QList<int> fileIndexList =
         m_project->getGroupFileIndexSet(groupIndex).toList();
      if (fileIndexList.size() < 1)
      {
         continue;
      }
      if (m_ui->tableWidget->rowCount() >= row)
      {
         m_ui->tableWidget->setRowCount(row + 100);
      }
      qSort(fileIndexList);
      foreach(int fileIndex, fileIndexList)
      {
         QString folderName = m_project->getFileFolderName(fileIndex);
         int column = columnList.indexOf(folderName);
         if (m_ui->tableWidget->item(row, column) != NULL)
         {
            column = -1;
         }
         if (column == -1)
         {
            column = columnList.size();
            columnList.append(folderName);
            if (m_ui->tableWidget->colorCount() >= columnList.size())
            {
               m_ui->tableWidget->setColumnCount(columnList.size() + 10);
            }
            m_ui->tableWidget->setHorizontalHeaderLabels(columnList);
         }
         QTableWidgetItem* fileItem = new QTableWidgetItem(m_project->getFileName(fileIndex));
         fileItem->setCheckState(m_project->isFileMoved(fileIndex) ? Qt::Checked : Qt::Unchecked);
         fileItem->setData(Qt::UserRole, fileIndex);
         m_ui->tableWidget->setItem(row, column, fileItem);
      }
      int firstFileIndex = fileIndexList.first();
      m_ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(
         QString(m_project->getFileMd4(firstFileIndex).toHex())));
      ++row;
   }
   m_ui->tableWidget->setRowCount(row);
   m_ui->tableWidget->setColumnCount(columnList.size());
   QHeaderView* horizontalHeader =  m_ui->tableWidget->horizontalHeader();
   horizontalHeader->setResizeMode(QHeaderView::ResizeToContents);

   QApplication::restoreOverrideCursor();
}

void MainWindow::showTablePreview(QTableWidgetItem* item)
{   
   int fileIndex = -1;
   if (item != NULL)
   {
      fileIndex = item->data(Qt::UserRole).toInt();
   }
   showPreview(fileIndex, m_ui->tablePreviewButton);
}

void MainWindow::showExternTablePreview()
{
   QTableWidgetItem* item = m_ui->tableWidget->currentItem();
   if (!item)
   {
      return;
   }
   int fileIndex = item->data(TREE_ROLE_FILE_INDEX).toInt();
   QString absoluteFilePath = m_project->getAbsoluteFilePath(fileIndex);
   absoluteFilePath.prepend("file://");
   QDesktopServices::openUrl(QUrl(absoluteFilePath));
}

void MainWindow::checkTableItemState(QTableWidgetItem* item)
{
   if (item)
   {
      int fileIndex = item->data(Qt::UserRole).toInt();
      m_project->setFileMoved(fileIndex, item->checkState() == Qt::Checked);
   }
}


void MainWindow::move()
{
   QList<int> fileIndexList = getSelectedFileList();
   foreach(int fileIndex, fileIndexList)
   {
      m_project->setFileMoved(fileIndex, true);
   }
}

void MainWindow::unmove()
{
   QList<int> fileIndexList = getSelectedFileList();
   foreach(int fileIndex, fileIndexList)
   {
      m_project->setFileMoved(fileIndex, false);
   }
}

void MainWindow::toggleMove()
{
   QList<int> fileIndexList = getSelectedFileList();
   foreach(int fileIndex, fileIndexList)
   {
      m_project->setFileMoved(fileIndex, !m_project->isFileMoved(fileIndex));
   }
}

QList<int> MainWindow::getSelectedFileList() const
{
   QList<QTableWidgetItem*> itemList = m_ui->tableWidget->selectedItems();
   QList<int> fileIndexList;
   foreach(QTableWidgetItem* item, itemList)
   {
      int fileIndex = item->data(Qt::UserRole).toInt();
      fileIndexList.append(fileIndex);
   }
   return fileIndexList;
}

void MainWindow::showPreview(int fileIndex, QPushButton* button)
{
   QString absoluteFilePath;
   if ((fileIndex == -1) || m_project->getFilePixmap(fileIndex).isNull())
   {
      button->setText("<no preview>");
   }
   else
   {
      button->setText("");
      absoluteFilePath = m_project->getAbsoluteFilePath(fileIndex);
   }
   QString styleSheetString;
   styleSheetString.append(QString("QPushButton {"));
   if (!absoluteFilePath.isEmpty())
   {
      styleSheetString.append(QString("image: url(%1);").arg(absoluteFilePath));
   }
   styleSheetString.append(QString("border: 2px solid #8f8f91;"));
   styleSheetString.append(QString("border-radius: 6px;"));
   styleSheetString.append(QString("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"));
   styleSheetString.append(QString("stop: 0 #f6f7fa, stop: 1 #dadbde);"));
   styleSheetString.append(QString("padding: 5px ;"));
   styleSheetString.append(QString("}"));

   styleSheetString.append(QString("QPushButton:pressed {"));
   styleSheetString.append(QString("   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"));
   styleSheetString.append(QString("stop: 0 #dadbde, stop: 1 #f6f7fa);"));
   styleSheetString.append(QString("            }"));

   styleSheetString.append(QString("QPushButton:flat {"));
   styleSheetString.append(QString("border: none; /* no border for a flat push button */"));
   styleSheetString.append(QString("            }"));

   styleSheetString.append(QString("QPushButton:default {"));
   styleSheetString.append(QString("            border-color: navy; /* make the default button prominent */"));
   styleSheetString.append(QString("            }"));
   button->setStyleSheet(styleSheetString);
}
