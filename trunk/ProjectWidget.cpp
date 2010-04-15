#include "ProjectWidget.h"
#include "ProjectWidgetItemDelegate.h"
#include "ProjectWidgetSortModel.h"
#include "ui_ProjectWidget.h"
#include "VCard.h"
#include "VCardProject.h"
#include "NewVCardDialog.h"
#include "NewTagDialog.h"

#include <QMessageBox>
#include <QScrollBar>
#include <QStandardItemModel>

ProjectWidget::ProjectWidget(VCardProject* project, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ProjectWidget),
    m_project(project),
    m_model(0),
    m_undoProject(0),
    m_redoProject(0),
    m_isProjectModified(false)
{
    m_ui->setupUi(this);

    m_ui->duplicatesTreeView->hide();

    m_ui->treeView->setItemDelegate(new ProjectWidgetItemDelegate(m_ui->treeView));

    updateProjectView();
    updateButtons();
}

void ProjectWidget::updateProjectView()
{
    int vScrollPos = m_ui->treeView->verticalScrollBar()->value();
    delete m_model;
    m_model = new ProjectWidgetSortModel(m_ui->treeView);

    QList<int> idList = m_project->getVCardIdList();
    QStandardItemModel* dataModel =
            new QStandardItemModel(idList.length(), COLUMN_COUNT, m_model);
    dataModel->setHeaderData(TAG_COLUMN, Qt::Horizontal, "Tag");
    dataModel->setHeaderData(PROPERTIES_COLUMN, Qt::Horizontal, "Properties");
    dataModel->setHeaderData(CONTENT_COLUMN, Qt::Horizontal, "Content");

    for(int row = 0; row < idList.size(); ++row)
    {
        int id = idList[row];
        VCard vCard = m_project->getVCard(id);

        QStandardItem* summaryTagItem = new QStandardItem(vCard.getSummary());
        summaryTagItem->setData(id, Qt::UserRole);
        summaryTagItem->setFlags(summaryTagItem->flags() & ~Qt::ItemIsEditable);
        summaryTagItem->setRowCount(vCard.getTagCount());
        summaryTagItem->setColumnCount(COLUMN_COUNT);
        dataModel->setItem(row, TAG_COLUMN, summaryTagItem);

        QStandardItem* summaryPropertiesItem = new QStandardItem();
        summaryPropertiesItem->setFlags(summaryPropertiesItem->flags() & ~Qt::ItemIsEditable);
        dataModel->setItem(row, PROPERTIES_COLUMN, summaryPropertiesItem);
        QStandardItem* summaryContentItem = new QStandardItem();
        summaryContentItem->setFlags(summaryContentItem->flags() & ~Qt::ItemIsEditable);
        dataModel->setItem(row, CONTENT_COLUMN, summaryContentItem);

        for(int tagIndex = 0; tagIndex < vCard.getTagCount(); ++tagIndex)
        {
            QString tag = vCard.getTag(tagIndex);
            QStandardItem* tagItem = new QStandardItem(tag);
            tagItem->setData(tagIndex, Qt::UserRole);
            if (!VCard::isTagEditable(tag))
            {
                tagItem->setForeground(Qt::gray);
            }
            summaryTagItem->setChild(tagIndex, TAG_COLUMN, tagItem);

            QStandardItem* propertyItem =
                    new QStandardItem(vCard.getTagProperties(tagIndex).join(";"));
            if (!VCard::isContentEditable(tag))
            {
                propertyItem->setForeground(Qt::gray);
            }
            summaryTagItem->setChild(tagIndex, PROPERTIES_COLUMN, propertyItem);

            QStandardItem* contentItem = new QStandardItem(vCard.getTagContent(tagIndex));
            if (!VCard::isContentEditable(tag))
            {
                contentItem->setForeground(Qt::gray);
            }
            summaryTagItem->setChild(tagIndex, CONTENT_COLUMN, contentItem);
        }
    }

    m_model->setSourceModel(dataModel);
    m_model->sort(TAG_COLUMN, Qt::AscendingOrder);

    //QHeaderView* header = m_ui->treeView->header();
    //header->setResizeMode(TAG_COLUMN, QHeaderView::ResizeToContents);
    m_ui->treeView->setModel(m_model);
    //header->setResizeMode(TAG_COLUMN, QHeaderView::Interactive);

    m_ui->treeView->expandAll();

    connect(m_ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(updateButtons()));
    connect(m_ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(updateTagInfo()));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateTagData()));

    m_ui->treeView->verticalScrollBar()->setValue(vScrollPos);
}

ProjectWidget::~ProjectWidget()
{
    delete m_ui;
}

const VCardProject& ProjectWidget::getProject() const
{
   return *m_project;
}

bool ProjectWidget::saveProject(QFile& file)
{
    if (m_project->saveTo(file))
    {
        m_isProjectModified = false;
        emit projectChanged();
        return true;
    }
    return false;
}

bool ProjectWidget::isProjectModified() const
{
    return m_isProjectModified;
}
bool ProjectWidget::canUndo() const
{
    return (m_undoProject != 0);
}
bool ProjectWidget::canRedo() const
{
    return (m_redoProject != 0);
}

void ProjectWidget::undo()
{
    if (m_undoProject != 0)
    {
        m_redoProject = m_project;
        m_project = m_undoProject;
        m_undoProject = 0;

        updateProjectView();
        m_isProjectModified  = true;
        emit projectChanged();
    }
}

void ProjectWidget::redo()
{
    if (m_redoProject != 0)
    {
        m_undoProject = m_project;
        m_project = m_redoProject;
        m_redoProject = 0;
        updateProjectView();

        updateProjectView();
        m_isProjectModified  = true;
        emit projectChanged();
    }
}

void ProjectWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ProjectWidget::on_expandButton_clicked()
{
    bool hasAllElementsExpanded = true;
    for(int row = 0; row < m_model->rowCount(); ++row)
    {
        QModelIndex tagColumnIndex = m_model->index(row, TAG_COLUMN);
        if (!m_ui->treeView->isExpanded(tagColumnIndex))
        {
            hasAllElementsExpanded = false;
            break;
        }
    }
    if (hasAllElementsExpanded)
    {
        m_ui->treeView->collapseAll();
    }
    else
    {
        m_ui->treeView->expandAll();
    }

}

void ProjectWidget::on_addVCardButton_clicked()
{
    NewVCardDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        VCard vCard;
        vCard.updateTag(-1, "VERSION", "2.1");
        QString fullName = dialog.getFullName();
        vCard.updateTag(-1, "FN", fullName);
        QString name = dialog.getName();
        vCard.updateTag(-1, "N", name);
        createUndoProject();
        m_project->addVCard(vCard);
        updateProjectView();
    }

    if (m_ui->duplicatesTreeView->isVisibleTo(this))
    {
        on_showDuplicatesButton_clicked();
    }
    updateButtons();

    m_isProjectModified  = true;
    emit projectChanged();
}

void ProjectWidget::on_removeVCardButton_clicked()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    createUndoProject();
    m_project->removeVCard(vCardId);
    updateProjectView();

    if (m_ui->duplicatesTreeView->isVisibleTo(this))
    {
        on_showDuplicatesButton_clicked();
    }
    updateButtons();

    m_isProjectModified  = true;
    emit projectChanged();
}

void ProjectWidget::on_insertTagButton_clicked()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    VCard vCard = m_project->getVCard(vCardId);
    int tagIndex = getTagIndex(currentIndex);
    QString tag = vCard.getTag(tagIndex);
    if ((tag.compare("BEGIN", Qt::CaseInsensitive) == 0) ||
        (tag.compare("VERSION", Qt::CaseInsensitive) == 0) ||
        (tag.compare("N", Qt::CaseInsensitive) == 0) ||
        (tag.compare("FN", Qt::CaseInsensitive) == 0))
    {
        QMessageBox::warning(this, "Warning",
                             QString("A new tag cannot be inserted at this position!").arg(tag));
        return;
    }
    NewTagDialog dialog(this);
    if(dialog.exec() == QDialog::Accepted)
    {
        vCard.insertTag(tagIndex);
        QString name = dialog.getName();
        QString properties = dialog.getProperties();
        QString completeTag = QString("%1;%2").arg(name).arg(properties);
        QString content = dialog.getContent();
        vCard.updateTag(tagIndex, completeTag, content);
        createUndoProject();
        m_project->updateVCard(vCardId, vCard);
        updateProjectView();
    }

    if (m_ui->duplicatesTreeView->isVisibleTo(this))
    {
        on_showDuplicatesButton_clicked();
    }
    updateButtons();

    m_isProjectModified  = true;
    emit projectChanged();
}

void ProjectWidget::on_removeTagButton_clicked()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    VCard vCard = m_project->getVCard(vCardId);
    int tagIndex = getTagIndex(currentIndex);
    QString tag = vCard.getTag(tagIndex);
    if ((tag.compare("BEGIN", Qt::CaseInsensitive) == 0) ||
        (tag.compare("END", Qt::CaseInsensitive) == 0) ||
        (tag.compare("VERSION", Qt::CaseInsensitive) == 0) ||
        (tag.compare("N", Qt::CaseInsensitive) == 0) ||
        (tag.compare("FN", Qt::CaseInsensitive) == 0))
    {
        QMessageBox::warning(this, "Warning", QString("Tag '%1' cannot be removed!").arg(tag));
        return;
    }
    vCard.removeTag(tagIndex);
    createUndoProject();
    m_project->updateVCard(vCardId, vCard);
    updateProjectView();

    if (m_ui->duplicatesTreeView->isVisibleTo(this))
    {
        on_showDuplicatesButton_clicked();
    }
    updateButtons();

    m_isProjectModified  = true;
    emit projectChanged();
}

void ProjectWidget::updateTagData()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    VCard vCard = m_project->getVCard(vCardId);
    int tagIndex = getTagIndex(currentIndex);
    QString oldTag = vCard.getTag(tagIndex);
    if (VCard::isTagEditable(oldTag))
    {
        QMessageBox::warning(this, "Warning",
                             QString("Tag '%1' cannot be changed!").arg(oldTag));
    }
    else
    {
        QModelIndex tagModelIndex =
                currentIndex.sibling(currentIndex.row(), TAG_COLUMN);
        QString tag = tagModelIndex.data().toString();
        QModelIndex propertiesModelIndex =
                currentIndex.sibling(currentIndex.row(), PROPERTIES_COLUMN);
        QString properties = propertiesModelIndex.data().toString();
        QString completeTag = QString("%1;%2").arg(tag).arg(properties);
        QModelIndex contentModelIndex =
                currentIndex.sibling(currentIndex.row(), CONTENT_COLUMN);
        QString content = contentModelIndex.data().toString();
        vCard.updateTag(tagIndex, completeTag, content);
        createUndoProject();
        m_project->updateVCard(vCardId, vCard);
    }

    updateProjectView();

    if (m_ui->duplicatesTreeView->isVisibleTo(this))
    {
        on_showDuplicatesButton_clicked();
    }
    updateButtons();

    m_isProjectModified  = true;
    emit projectChanged();
}

void ProjectWidget::updateButtons()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();

    m_ui->addVCardButton->setEnabled(true);
    m_ui->removeVCardButton->setEnabled(currentIndex.isValid());
    m_ui->insertTagButton->setEnabled(currentIndex.isValid() &&
                                      currentIndex.parent().isValid());
    m_ui->removeTagButton->setEnabled(currentIndex.isValid() &&
                                      currentIndex.parent().isValid());
}

void ProjectWidget::updateTagInfo()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    VCard vCard = m_project->getVCard(vCardId);
    int tagIndex = getTagIndex(currentIndex);
    QString tag = vCard.getTag(tagIndex);
    m_ui->infoTextEdit->setHtml(VCard::getTagInfo(tag));
}

void ProjectWidget::showSelectedDuplicates()
{
    m_model->blockSignals(true);
    for(int vCardRow = 0; vCardRow < m_model->rowCount(); ++vCardRow)
    {
        QModelIndex vCardIndex = m_model->index(vCardRow, TAG_COLUMN);
        for (int tagRow = 0; tagRow < m_model->rowCount(vCardIndex); ++tagRow)
        {
            QModelIndex tagContentIndex =
                    m_model->index(tagRow, CONTENT_COLUMN, vCardIndex);
            if (tagContentIndex.data(Qt::ForegroundRole) != Qt::black)
            {
                m_model->setData(tagContentIndex, Qt::black, Qt::ForegroundRole);
            }
        }
    }
    m_model->blockSignals(false);

    QModelIndex currentIndex = m_ui->duplicatesTreeView->currentIndex();
    if (!currentIndex.isValid())
    {
        m_model->setFilteredContent("");
        return;
    }
    QModelIndex contentIndex= currentIndex.sibling(currentIndex.row(), 0);
    QString content = contentIndex.data(Qt::UserRole).toString();
    m_model->setFilteredContent(content);
    if (!content.isEmpty())
    {
        m_ui->treeView->expandAll();
    }

    m_model->blockSignals(true);
    for(int vCardRow = 0; vCardRow < m_model->rowCount(); ++vCardRow)
    {
        QModelIndex vCardIndex = m_model->index(vCardRow, TAG_COLUMN);
        for (int tagRow = 0; tagRow < m_model->rowCount(vCardIndex); ++tagRow)
        {
            QModelIndex tagContentIndex =
                    m_model->index(tagRow, CONTENT_COLUMN, vCardIndex);
            if (tagContentIndex.data().toString() == content)
            {
                m_model->setData(tagContentIndex, Qt::red, Qt::ForegroundRole);
            }
        }
    }
    m_model->blockSignals(false);
}

int ProjectWidget::getVCardId(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return -1;
    }
    QModelIndex currentIndex = index.sibling(index.row(), TAG_COLUMN);
    if (currentIndex.parent().isValid())
    {
        currentIndex = currentIndex.parent();
    }
    int vCardId = currentIndex.data(Qt::UserRole).toInt();
    return vCardId;
}

int ProjectWidget::getTagIndex(const QModelIndex& index) const\
{
    if (!index.isValid())
    {
        return -1;
    }
    QModelIndex currentIndex = index.sibling(index.row(), TAG_COLUMN);
    if (!currentIndex.parent().isValid())
    {
        return -1;
    }
    int tagIndex = currentIndex.data(Qt::UserRole).toInt();
    return tagIndex;

}

void ProjectWidget::createUndoProject()
{
    delete m_redoProject;
    m_redoProject = 0;
    delete m_undoProject;
    m_undoProject = new VCardProject(*m_project);
}

void ProjectWidget::on_showDuplicatesButton_clicked()
{
    QMap<QString, int> duplicatesMap;

    QList<int> idList = m_project->getVCardIdList();

    QString oldSelectedContent;
    if (m_ui->duplicatesTreeView->currentIndex().isValid())
    {
        oldSelectedContent = m_ui->duplicatesTreeView->currentIndex()
                             .data(Qt::UserRole).toString();
    }
    int oldScrollPos = m_ui->duplicatesTreeView->verticalScrollBar()->value();

    for(int row = 0; row < idList.size(); ++row)
    {
        int id = idList[row];
        VCard vCard = m_project->getVCard(id);
        for(int tagIndex = 0; tagIndex < vCard.getTagCount(); ++tagIndex)
        {
            QString tag = vCard.getTag(tagIndex);
            if (!VCard::isContentEditable(tag))
            {
                continue;
            }
            QString content = vCard.getTagContent(tagIndex);
            if (duplicatesMap.contains(content))
            {
                duplicatesMap[content]++;
            }
            else
            {
                duplicatesMap[content] = 1;
            }
        }
    }
    foreach(QString content, duplicatesMap.keys())
    {
        if ((duplicatesMap[content] == 1) && (content != oldSelectedContent))
        {
            duplicatesMap.remove(content);
        }
    }

    QStandardItemModel* duplicatesModel = new QStandardItemModel(duplicatesMap.size(),
            DUP_COLUMN_COUNT, m_ui->duplicatesTreeView);
    duplicatesModel->setHorizontalHeaderItem(DUP_TEXT_COLUMN,
                                             new QStandardItem("Text"));
    duplicatesModel->setHorizontalHeaderItem(DUP_COUNT_COLUMN,
                                             new QStandardItem("Count"));
    QList<QString> duplicatesList = duplicatesMap.keys();
    for(int row = 0; row < duplicatesList.length(); ++row)
    {
        QString content = duplicatesList[row];
        QStandardItem* contentItem = new QStandardItem(content.simplified());
        contentItem->setData(content, Qt::UserRole);
        duplicatesModel->setItem(row, DUP_TEXT_COLUMN, contentItem);
        QStandardItem* countItem = new QStandardItem(
                QString::number(duplicatesMap[content]));
        duplicatesModel->setItem(row, DUP_COUNT_COLUMN, countItem);
    }
    QStandardItem* showAllItem = new QStandardItem("Show all entries");
    duplicatesModel->insertRow(0);
    duplicatesModel->setItem(0, showAllItem);

    m_ui->duplicatesTreeView->setModel(duplicatesModel);
    m_ui->duplicatesTreeView->setCurrentIndex(
            duplicatesModel->index(0, DUP_TEXT_COLUMN));
    for(int row = 0; row < duplicatesModel->rowCount(); ++row)
    {
        QModelIndex index = duplicatesModel->index(row, DUP_TEXT_COLUMN);
        if (index.data(Qt::UserRole).toString() == oldSelectedContent)
        {
            m_ui->duplicatesTreeView->setCurrentIndex(index);
            m_ui->duplicatesTreeView->verticalScrollBar()->setValue(oldScrollPos);
            break;
        }
    }
    m_ui->duplicatesTreeView->header()->setResizeMode(
            DUP_TEXT_COLUMN, QHeaderView::Stretch);

    m_ui->duplicatesTreeView->show();
    m_ui->duplicatesButtonStackedWidget->setCurrentWidget(m_ui->hideDuplicatesPage);

    showSelectedDuplicates();

    connect(m_ui->duplicatesTreeView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(showSelectedDuplicates()));
}

void ProjectWidget::on_hideDuplicatesButton_clicked()
{
    QStandardItemModel* duplicatesModel =
            new QStandardItemModel(1, DUP_COLUMN_COUNT, m_ui->duplicatesTreeView);
    QStandardItem* showAllItem = new QStandardItem("Show all entries");
    duplicatesModel->setItem(0, showAllItem);

    m_ui->duplicatesTreeView->setModel(duplicatesModel);
    m_ui->duplicatesTreeView->setCurrentIndex(
            duplicatesModel->index(0, DUP_TEXT_COLUMN));

    m_ui->duplicatesTreeView->hide();
    m_ui->duplicatesButtonStackedWidget->setCurrentWidget(m_ui->showDuplicatesPage);

    showSelectedDuplicates();
}
