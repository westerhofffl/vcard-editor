#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Project.h"
#include "ProjectSettingsDialog.h"

#include <QButtonGroup>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QImage>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        m_ui(new Ui::MainWindow),
        m_project(0),
        m_tableGroupIndex(-1),
        m_tableFileIndex(-1),
        m_totalGroupCount(0),
        m_duplicatesGroupCount(0),
        m_notResolvedGroupCount(0),
        m_removedGroupCount(0)
{
    m_ui->setupUi(this);
    setWindowTitle("Duplicates detector");

    m_ui->filesGroupBox->setEnabled(false);
    m_ui->foldersGroupBox->setEnabled(false);
    m_ui->pauseButton->setEnabled(false);

    m_ui->tableWidget->horizontalHeader()->setMovable(true);

    connect(m_ui->actionNew, SIGNAL(triggered()), SLOT(createNewProject()));
    connect(m_ui->pauseButton, SIGNAL(clicked()), SLOT(startStopProject()));

    m_showButtonGroup = new QButtonGroup(this);
    m_showButtonGroup->addButton(m_ui->showAllRadioButton, SHOW_ALL);
    m_showButtonGroup->addButton(m_ui->showDuplicatesOnlyRadioButton, SHOW_DUPLICATED_ONLY);
    m_showButtonGroup->addButton(m_ui->showNotResolvedOnlyRadioButton, SHOW_NOT_RESOLVED_ONLY);
    m_showButtonGroup->addButton(m_ui->showRemovedOnlyRadioButton, SHOW_REMOVED_ONLY);
    connect(m_showButtonGroup, SIGNAL(buttonClicked(int)), SLOT(updateFilter()));

    connect(m_ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(showTreePreview(QTreeWidgetItem*)));

    connect(m_ui->treePreviewButton, SIGNAL(clicked()), SLOT(showExternTreePreview()));

    connect(m_ui->tableWidget, SIGNAL(itemSelectionChanged()),
            SLOT(showTablePreview()));

    connect(m_ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
            SLOT(checkTableItemState(QTableWidgetItem*)));

    connect(m_ui->tablePreviewButton, SIGNAL(clicked()), SLOT(showExternTablePreview()));

    connect(m_ui->moveButton, SIGNAL(clicked()), SLOT(move()));
    connect(m_ui->unmoveButton, SIGNAL(clicked()), SLOT(unmove()));
    connect(m_ui->toggleMoveButton, SIGNAL(clicked()), SLOT(toggleMove()));

    m_ui->treeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

    showTreePreview(NULL);
    showTablePreview();
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

        m_ui->filesGroupBox->setEnabled(true);
        m_ui->foldersGroupBox->setEnabled(true);
        m_ui->pauseButton->setEnabled(true);

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
        if (progress == 100)
        {
            m_ui->pauseButton->setText("Ready");
            m_ui->pauseButton->setEnabled(false);
        }
    }
}


void MainWindow::updateGroup(int index)
{
    QTreeWidgetItem* groupItem = NULL;
    bool wasMovedGroup = false;
    bool wasDuplicatesGroup = false;
    bool wasNotResolvedGroup = false;

    if (m_ui->treeWidget->topLevelItemCount() <= index)
    {
        groupItem = new QTreeWidgetItem();
    }
    else
    {
        groupItem = m_ui->treeWidget->topLevelItem(index);
        int fileCount = groupItem->data(0, TREE_ROLE_FILE_COUNT).toInt();
        int movedCount = groupItem->data(0, TREE_ROLE_MOVED_COUNT).toInt();

        wasMovedGroup = (fileCount == movedCount);
        wasDuplicatesGroup = (fileCount > 1);
        wasNotResolvedGroup = (fileCount - movedCount > 1);
    }
    QList<int> fileIndexList = m_project->getGroupFileIndexSet(index).toList();
    qSort(fileIndexList);
    QSet<QString> fileNameSet;
    foreach(int fileIndex, fileIndexList)
    {
        fileNameSet.insert(m_project->getFileName(fileIndex));
    }
    QStringList fileNameList = fileNameSet.toList();
    qSort(fileNameList);

    QString groupItemText;
    int maxCount = 0;
    QMap<QString, int> fileNameCountMap;
    foreach(const QString& fileName, fileNameList)
    {
        int count = fileNameCountMap.value(fileName, 1) + 1;
        fileNameCountMap[fileName] = count;
        if (maxCount < count)
        {
            groupItemText = fileName;
            maxCount = count;
        }
    }
    if (fileNameList.size() > 1)
    {
        groupItemText.append(",...");
    }
    groupItem->setText(0, groupItemText);
    int firstFileIndex = fileIndexList.first();
    groupItem->setData(0, TREE_ROLE_FILE_INDEX, firstFileIndex);
    groupItem->setData(0, TREE_ROLE_GROUP_INDEX, index);
    groupItem->setData(0, TREE_ROLE_FILE_COUNT, fileIndexList.size());
    groupItem->setText(2, QString::number(m_project->getFileSize(firstFileIndex)));
    QByteArray md4 = m_project->getFileCheckSum(firstFileIndex);
    groupItem->setText(3, m_project->getFileCheckSum(firstFileIndex).toHex());

    int movedCount = 0;
    for(int row = 0; row < fileIndexList.size(); ++row)
    {
        QTreeWidgetItem* fileItem = groupItem->child(row);
        if (fileItem == NULL)
        {
            fileItem = new QTreeWidgetItem(groupItem);
        }
        int fileIndex = fileIndexList[row];
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

    //updateTable(-1, -1);
    updateFilter(groupItem);

    bool isMovedGroup = (fileIndexList.size() == movedCount);
    bool isDuplicatesGroup = (fileIndexList.size() > 1);
    bool isNotResolvedGroup = (fileIndexList.size() - movedCount > 1);

    if (m_ui->treeWidget->topLevelItemCount() <= index)
    {
        m_ui->treeWidget->addTopLevelItem(groupItem);
        ++m_totalGroupCount;
        if (isMovedGroup)
        {
            ++m_removedGroupCount;
        }
        if (isDuplicatesGroup)
        {
            ++m_duplicatesGroupCount;
        }
        if (isNotResolvedGroup)
        {
            ++m_notResolvedGroupCount;
        }
    }
    else
    {
        m_removedGroupCount += (isMovedGroup != wasMovedGroup) * (isMovedGroup ? 1 : -1);
        m_duplicatesGroupCount += (isDuplicatesGroup != wasDuplicatesGroup) * (isDuplicatesGroup ? 1 : -1);
        m_notResolvedGroupCount += (isNotResolvedGroup != wasNotResolvedGroup) * (isNotResolvedGroup ? 1 : -1);
    }
    m_ui->showAllRadioButton->setText(QString("all (%1)").arg(m_totalGroupCount));
    m_ui->showDuplicatesOnlyRadioButton->setText(QString("duplicates only (%1)").arg(m_duplicatesGroupCount));
    m_ui->showNotResolvedOnlyRadioButton->setText(QString("not resolved only (%1)").arg(m_notResolvedGroupCount));
    m_ui->showRemovedOnlyRadioButton->setText(QString("removed only (%1)").arg(m_removedGroupCount));
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
    switch(m_showButtonGroup->checkedId())
    {
    case SHOW_ALL:
        item->setHidden(false);
        break;
    case SHOW_DUPLICATED_ONLY:
        item->setHidden(fileCount <= 1);
        break;
    case SHOW_NOT_RESOLVED_ONLY:
        item->setHidden((fileCount <= 1) || ((fileCount - movedCount) <= 1));
        break;
    case SHOW_REMOVED_ONLY:
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(absoluteFilePath));
}

void MainWindow::updateTable(int selectedGroupIndex, int selectedFileIndex)
{
    if (selectedGroupIndex == -1 && selectedFileIndex == -1)
    {
        if (m_tableGroupIndex == -1 && m_tableFileIndex == -1)
        {
            return;
        }
        selectedGroupIndex = m_tableGroupIndex;
        selectedFileIndex = m_tableFileIndex;
    }
    else
    {
        m_tableGroupIndex = selectedGroupIndex;
        m_tableFileIndex = selectedFileIndex;
    }

    m_ui->tableWidget->viewport()->setUpdatesEnabled(false);
    m_ui->tableWidget->blockSignals(true);

    //QApplication::setOverrideCursor(Qt::WaitCursor);
    int currentFileIndex = -1;
    QTableWidgetItem* currentItem = m_ui->tableWidget->currentItem();
    if (currentItem != NULL)
    {
        currentFileIndex = currentItem->data(Qt::UserRole).toInt();
    }
    QSet<int> selectedFileIndexSet;
    foreach(QTableWidgetItem* selectedItem, m_ui->tableWidget->selectedItems())
    {
        QVariant dataVariant = selectedItem->data(Qt::UserRole);
        int currentFileIndex = dataVariant.toInt();
        selectedFileIndexSet.insert(currentFileIndex);
    }

    m_ui->tableWidget->clear();
    m_ui->tableWidget->setRowCount(0);
    m_ui->tableWidget->setColumnCount(0);
    QCoreApplication::processEvents();

    m_ui->tableWidget->setRowCount(100);
    m_ui->tableWidget->setColumnCount(10);

    QList<int> selectedFileList;
    if (selectedFileIndex != -1)
    {
        selectedFileList.append(selectedFileIndex);
    }
    else
    {
        selectedFileList = m_project->getGroupFileIndexSet(selectedGroupIndex).toList();
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
            while ((column != -1) && (m_ui->tableWidget->item(row, column) != NULL))
            {
                column = columnList.indexOf(folderName, column + 1);
            }
            if (column == -1)
            {
                column = columnList.size();
                columnList.append(folderName);
                if (m_ui->tableWidget->columnCount() >= columnList.size())
                {
                    m_ui->tableWidget->setColumnCount(columnList.size() + 10);
                }
            }
            QTableWidgetItem* fileItem = new QTableWidgetItem(m_project->getFileName(fileIndex));
            qWarning("QTableWidgetItem for %s, checked: %i", m_project->getFileName(fileIndex).toAscii().data(), m_project->isFileMoved(fileIndex));
            fileItem->setCheckState(m_project->isFileMoved(fileIndex) ? Qt::Checked : Qt::Unchecked);
            fileItem->setData(Qt::UserRole, fileIndex);
            if (((groupIndex == selectedGroupIndex) && (selectedFileIndex == -1)) ||
                (fileIndex == selectedFileIndex))
            {
                QFont font = fileItem->font();
                font.setBold(true);
                fileItem->setFont(font);
            }
            m_ui->tableWidget->setItem(row, column, fileItem);
            if (fileIndex == currentFileIndex)
            {
                currentItem = fileItem;
            }
            if (selectedFileIndexSet.contains(fileIndex))
            {
                fileItem->setSelected(true);
            }
        }
        int firstFileIndex = fileIndexList.first();
        m_ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(
                QString(m_project->getFileCheckSum(firstFileIndex).toHex())));
        ++row;
    }
    m_ui->tableWidget->setRowCount(row);
    m_ui->tableWidget->setColumnCount(columnList.size());
    for(int column = 0; column < columnList.size(); ++column)
    {
        int itemCount = 0;
        for(int itemRow = 0; itemRow < row; ++itemRow)
        {
            if (m_ui->tableWidget->item(itemRow, column) != NULL)
            {
                itemCount++;
            }
        }
        QString columnName = columnList[column];
        if (columnName.startsWith("/"))
        {
            columnName = columnName.mid(1);
        }
        columnName.replace("/", "\n/");
        columnName.prepend("/");
        columnName.append(QString("\n(%1 pics)")
                             .arg(itemCount));
        QTableWidgetItem* headerItem =
                new QTableWidgetItem(columnName);
        QFont font(headerItem->font());
        font.setPixelSize(10);
        headerItem->setFont(font);
        m_ui->tableWidget->setHorizontalHeaderItem(column,
                                                   headerItem);
    }
    QHeaderView* horizontalHeader =  m_ui->tableWidget->horizontalHeader();
    horizontalHeader->setResizeMode(QHeaderView::Interactive);
    m_ui->tableWidget->resizeColumnsToContents();

    m_ui->tableWidget->viewport()->setUpdatesEnabled(true);
    m_ui->tableWidget->blockSignals(false);
    m_ui->tableWidget->setCurrentItem(currentItem,QItemSelectionModel::NoUpdate);

    //QApplication::restoreOverrideCursor();
}

void MainWindow::showTablePreview()
{
    int fileIndex = -1;
    QList<QTableWidgetItem*> itemList = m_ui->tableWidget->selectedItems();
    if (itemList.size() == 1)
    {
        fileIndex = itemList.first()->data(Qt::UserRole).toInt();
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(absoluteFilePath));
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
    updateTable(-1, -1);
}

void MainWindow::unmove()
{
    QList<int> fileIndexList = getSelectedFileList();
    foreach(int fileIndex, fileIndexList)
    {
        m_project->setFileMoved(fileIndex, false);
    }
    updateTable(-1, -1);
}

void MainWindow::toggleMove()
{
    QList<int> fileIndexList = getSelectedFileList();
    foreach(int fileIndex, fileIndexList)
    {
        m_project->setFileMoved(fileIndex, !m_project->isFileMoved(fileIndex));
    }
    updateTable(-1, -1);
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
