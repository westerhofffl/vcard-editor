#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Project.h"
#include "ProjectSettingsDialog.h"

#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QImage>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_project(0)
{
    m_ui->setupUi(this);

    connect(m_ui->actionNew, SIGNAL(triggered()), SLOT(createNewProject()));
    connect(m_ui->pauseButton, SIGNAL(clicked()), SLOT(startStopProject()));

    connect(m_ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(showTreePreview(QTreeWidgetItem*)));
    connect(m_ui->tableWidget, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
       SLOT(showTablePreview(QTableWidgetItem*)));
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
      m_project = new Project(settingsDialog->getFolderName());
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
   }
   else
   {
      m_ui->progressBar->setRange(0, 100);
      m_ui->progressBar->setValue(progress);
   }
   m_ui->statusLabel->setText(text);
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
   groupItem->setData(0, Qt::UserRole, firstFileIndex);
   groupItem->setData(0, Qt::UserRole + 1, index);
   groupItem->setText(1, QString::number(fileIndexList.size()));
   groupItem->setText(2, QString::number(m_project->getFileSize(firstFileIndex)));
   QByteArray md4 = m_project->getFileMd4(firstFileIndex);
   groupItem->setText(3, m_project->getFileMd4(firstFileIndex).toHex());

   qDeleteAll(groupItem->takeChildren());
   foreach(int fileIndex, fileIndexList)
   {
      QTreeWidgetItem* fileItem = new QTreeWidgetItem(groupItem);
      QFileInfo fileInfo(m_project->getFileFolderName(fileIndex), m_project->getFileName(fileIndex));
      fileItem->setText(0, fileInfo.absoluteFilePath());
      fileItem->setData(0, Qt::UserRole, fileIndex);
   }

   groupItem->setHidden(fileIndexList.size() == 1);
}

void MainWindow::showTreePreview(QTreeWidgetItem* item)
{
   int fileIndex = item->data(0, Qt::UserRole).toInt();
   QFileInfo fileInfo(m_project->getFileFolderName(fileIndex), m_project->getFileName(fileIndex));
   QImage image(fileInfo.absoluteFilePath());
   QSize labelSize = m_ui->treePreviewLabel->size();
   m_ui->treePreviewLabel->setPixmap(QPixmap::fromImage(image).scaled(labelSize, Qt::KeepAspectRatio));

   QVariant groupIndexVariant = item->data(0, Qt::UserRole + 1);
   if (!groupIndexVariant.isValid())
   {
      groupIndexVariant = item->parent()->data(0, Qt::UserRole + 1);
   }
   else
   {
      fileIndex = -1;
   }

   int groupIndex = groupIndexVariant.toInt();

   updateTable(groupIndex, fileIndex);
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
      if (fileIndexList.size() <= 1)
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
         fileItem->setCheckState(Qt::Unchecked);
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
   QApplication::restoreOverrideCursor();
}

void MainWindow::showTablePreview(QTableWidgetItem* item)
{
   if (item == 0)
   {
      m_ui->tablePreviewLabel->setPixmap(QPixmap());
   }
   else
   {
      int fileIndex = item->data(Qt::UserRole).toInt();
      QFileInfo fileInfo(m_project->getFileFolderName(fileIndex), m_project->getFileName(fileIndex));
      QImage image(fileInfo.absoluteFilePath());
      QSize labelSize = m_ui->tablePreviewLabel->size();
      m_ui->tablePreviewLabel->setPixmap(QPixmap::fromImage(image).scaled(labelSize, Qt::KeepAspectRatio));
   }
}
