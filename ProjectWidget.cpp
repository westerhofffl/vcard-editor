#include "ProjectWidget.h"
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
    m_model(0)
{
    m_ui->setupUi(this);

    m_ui->duplicatesTreeView->hide();

    updateProjectView();
    updateButtons();
}

void ProjectWidget::updateProjectView()
{
    int vScrollPos = m_ui->treeView->verticalScrollBar()->value();
    delete m_model;
    m_model = new SortModel(m_ui->treeView);

    QList<int> idList = m_project->getVCardIdList();
    QStandardItemModel* dataModel = new QStandardItemModel(idList.length(), COLUMN_COUNT, m_model);
    dataModel->setHeaderData(TAG_COLUMN, Qt::Horizontal, "Tag");
    dataModel->setHeaderData(PROPERTIES_COLUMN, Qt::Horizontal, "Properties");
    dataModel->setHeaderData(CONTENT_COLUMN, Qt::Horizontal, "Content");

    for(int row = 0; row < idList.size(); ++row)
    {
        int id = idList[row];
        VCard vCard = m_project->getVCard(id);

        QStandardItem* item = new QStandardItem(vCard.getSummary());
        item->setData(id, Qt::UserRole);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setRowCount(vCard.getTagCount());
        item->setColumnCount(COLUMN_COUNT);
        dataModel->setItem(row, item);

        for(int tagIndex = 0; tagIndex < vCard.getTagCount(); ++tagIndex)
        {
            QStandardItem* tagItem = new QStandardItem(vCard.getTag(tagIndex));
            tagItem->setData(tagIndex, Qt::UserRole);
            item->setChild(tagIndex, TAG_COLUMN, tagItem);
            QStandardItem* propertyItem = new QStandardItem(vCard.getTagProperties(tagIndex).join(";"));
            item->setChild(tagIndex, PROPERTIES_COLUMN, propertyItem);
            QStandardItem* contentItem = new QStandardItem(vCard.getTagContent(tagIndex));
            item->setChild(tagIndex, CONTENT_COLUMN, contentItem);
        }        
    }

    m_model->setSourceModel(dataModel);
    m_model->sort(TAG_COLUMN, Qt::AscendingOrder);

    //QHeaderView* header = m_ui->treeView->header();
    //header->setResizeMode(TAG_COLUMN, QHeaderView::ResizeToContents);
    m_ui->treeView->setModel(m_model);
    //header->setResizeMode(TAG_COLUMN, QHeaderView::Interactive);

    m_ui->treeView->expandAll();

    connect(m_ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(updateButtons()));
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

///

ProjectWidget::SortModel::SortModel(QObject* parent) :
        QSortFilterProxyModel(parent)
{

}

void ProjectWidget::SortModel::setFilteredContent(const QString& content)
{
    m_filteredContent = content;
    invalidateFilter();
}

bool ProjectWidget::SortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (!left.parent().isValid() && !right.parent().isValid())
    {
        QModelIndex leftTagIndex = left.sibling(TAG_COLUMN, left.column());
        QModelIndex rightTagIndex = right.sibling(TAG_COLUMN, right.column());
        return (leftTagIndex.data().toString() < rightTagIndex.data().toString());
    }
    if (left.parent().isValid() && right.parent().isValid())
    {
        QModelIndex leftTagIndex = left.sibling(TAG_COLUMN, left.column());
        QModelIndex rightTagIndex = right.sibling(TAG_COLUMN, right.column());
        return (leftTagIndex.data(Qt::UserRole).toInt() < rightTagIndex.data(Qt::UserRole).toInt());
        if (leftTagIndex.data().toString() < rightTagIndex.data().toString())
        {
            return true;
        }
        if (leftTagIndex.data().toString() > rightTagIndex.data().toString())
        {
            return true;
        }
        QModelIndex leftPropertiesIndex = left.sibling(PROPERTIES_COLUMN, left.column());
        QModelIndex rightPropertiesIndex = right.sibling(PROPERTIES_COLUMN, right.column());
        return (leftPropertiesIndex.data(Qt::UserRole).toInt() < rightPropertiesIndex.data(Qt::UserRole).toInt());
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


bool ProjectWidget::SortModel::filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const
{
    return true;
}
bool ProjectWidget::SortModel::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
    if (m_filteredContent.isEmpty())
    {
        return true;
    }
    if (source_parent.isValid())
    {
        return true;
    }

    QAbstractItemModel* dataModel = sourceModel();
    QModelIndex vCardIndex = dataModel->index(source_row, TAG_COLUMN);
    for (int childRow = 0; childRow < dataModel->rowCount(vCardIndex); ++childRow)
    {
        QModelIndex contentIndex = dataModel->index(childRow, CONTENT_COLUMN, vCardIndex);
        QString content = contentIndex.data().toString();
        if (content == m_filteredContent)
        {
            return true;
        }
    }
    return false;
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
        m_project->addVCard(vCard);
        updateProjectView();
    }
}

void ProjectWidget::on_removeVCardButton_clicked()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    m_project->removeVCard(vCardId);
    updateProjectView();
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
        QMessageBox::warning(this, "Warning", QString("A new tag cannot be inserted at this position!").arg(tag));
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
        m_project->updateVCard(vCardId, vCard);
        updateProjectView();
    }
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
    m_project->updateVCard(vCardId, vCard);
    updateProjectView();
}

void ProjectWidget::updateTagData()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    int vCardId = getVCardId(currentIndex);
    VCard vCard = m_project->getVCard(vCardId);
    int tagIndex = getTagIndex(currentIndex);
    QString oldTag = vCard.getTag(tagIndex);
    if ((oldTag.compare("BEGIN", Qt::CaseInsensitive) == 0) ||
        (oldTag.compare("END", Qt::CaseInsensitive) == 0) ||
        (oldTag.compare("VERSION", Qt::CaseInsensitive) == 0))
    {
        QMessageBox::warning(this, "Warning", QString("Tag '%1' cannot be changed!").arg(oldTag));
    }
    else
    {
        QString tag = currentIndex.sibling(currentIndex.row(), TAG_COLUMN).data().toString();
        QString properties = currentIndex.sibling(currentIndex.row(), PROPERTIES_COLUMN).data().toString();
        QString completeTag = QString("%1;%2").arg(tag).arg(properties);
        QString content = currentIndex.sibling(currentIndex.row(), CONTENT_COLUMN).data().toString();
        vCard.updateTag(tagIndex, completeTag, content);
        m_project->updateVCard(vCardId, vCard);
    }

    updateProjectView();
}

void ProjectWidget::updateButtons()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();

    m_ui->addVCardButton->setEnabled(true);
    m_ui->removeVCardButton->setEnabled(currentIndex.isValid());
    m_ui->insertTagButton->setEnabled(currentIndex.isValid());
    m_ui->removeTagButton->setEnabled(currentIndex.isValid());
}

void ProjectWidget::showSelectedDuplicates()
{
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

void ProjectWidget::on_showDuplicatesButton_clicked()
{
    //QMultiHash<content, QPair<vCardId, tagIndex> >
    QMap<QString, int> duplicatesMap;

    QList<int> idList = m_project->getVCardIdList();

    for(int row = 0; row < idList.size(); ++row)
    {
        int id = idList[row];
        VCard vCard = m_project->getVCard(id);
        for(int tagIndex = 0; tagIndex < vCard.getTagCount(); ++tagIndex)
        {
            QString tag = vCard.getTag(tagIndex);
            if ((tag.compare("BEGIN", Qt::CaseInsensitive) == 0) ||
                (tag.compare("END", Qt::CaseInsensitive) == 0) ||
                (tag.compare("VERSION", Qt::CaseInsensitive) == 0))
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
        if (duplicatesMap[content] == 1)
        {
            duplicatesMap.remove(content);
        }
    }

    QStandardItemModel* duplicatesModel = new QStandardItemModel(duplicatesMap.size(), 2, m_ui->duplicatesTreeView);
    QList<QString> duplicatesList = duplicatesMap.keys();
    for(int row = 0; row < duplicatesList.length(); ++row)
    {
        QString content = duplicatesList[row];
        QStandardItem* contentItem = new QStandardItem(content.simplified());
        contentItem->setData(content, Qt::UserRole);
        duplicatesModel->setItem(row, 0, contentItem);
        QStandardItem* countItem = new QStandardItem(QString::number(duplicatesMap[content]));
        duplicatesModel->setItem(row, 1, countItem);
    }
    QStandardItem* showAllItem = new QStandardItem("Show all entries");
    duplicatesModel->insertRow(0);
    duplicatesModel->setItem(0, showAllItem);

    m_ui->duplicatesTreeView->setModel(duplicatesModel);
    m_ui->duplicatesTreeView->setCurrentIndex(duplicatesModel->index(0, 0));
    m_ui->duplicatesTreeView->header()->setResizeMode(0, QHeaderView::Stretch);

    m_ui->duplicatesTreeView->show();
    m_ui->duplicatesButtonStackedWidget->setCurrentWidget(m_ui->hideDuplicatesPage);

    showSelectedDuplicates();

    connect(m_ui->duplicatesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(showSelectedDuplicates()));
}

void ProjectWidget::on_hideDuplicatesButton_clicked()
{
    QStandardItemModel* duplicatesModel = new QStandardItemModel(1, 2, m_ui->duplicatesTreeView);
    QStandardItem* showAllItem = new QStandardItem("Show all entries");
    duplicatesModel->setItem(0, showAllItem);    

    m_ui->duplicatesTreeView->setModel(duplicatesModel);
    m_ui->duplicatesTreeView->setCurrentIndex(duplicatesModel->index(0, 0));

    m_ui->duplicatesTreeView->hide();
    m_ui->duplicatesButtonStackedWidget->setCurrentWidget(m_ui->showDuplicatesPage);

    showSelectedDuplicates();
}
