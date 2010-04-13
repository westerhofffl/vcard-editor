#include "ProjectWidget.h"
#include "ui_ProjectWidget.h"
#include "VCard.h"
#include "VCardProject.h"

#include <QStandardItemModel>

ProjectWidget::ProjectWidget(VCardProject* project, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ProjectWidget),
    m_project(project),
    m_model(0)
{
    m_ui->setupUi(this);

    updateProjectView();
}

void ProjectWidget::updateProjectView()
{
    delete m_model;
    m_model = new SortModel(m_ui->treeView);

    QStandardItemModel* dataModel = new QStandardItemModel(m_project->getVCardCount(), COLUMN_COUNT, m_model);
    dataModel->setHeaderData(TAG_COLUMN, Qt::Horizontal, "Tag");
    dataModel->setHeaderData(PROPERTIES_COLUMN, Qt::Horizontal, "Properties");
    dataModel->setHeaderData(CONTENT_COLUMN, Qt::Horizontal, "Content");
    m_model->setSourceModel(dataModel);    

    for(int index = 0; index < m_project->getVCardCount(); ++index)
    {
        VCard vCard = m_project->getVCard(index);

        QStandardItem* item = new QStandardItem(vCard.getSummary());
        item->setData(index, Qt::UserRole);
        item->setRowCount(vCard.getTagCount());
        item->setColumnCount(COLUMN_COUNT);
        dataModel->setItem(index, item);

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

    m_model->sort(TAG_COLUMN, Qt::AscendingOrder);
    m_ui->treeView->setModel(m_model);

    m_ui->treeView->expandAll();
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

bool ProjectWidget::SortModel::lessThan(const QModelIndex &left, const QModelIndex &right)
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
        return (leftPropertiesIndex.data().toString() < rightPropertiesIndex.data().toString());
    }
    return QSortFilterProxyModel::lessThan(left, right);
}
