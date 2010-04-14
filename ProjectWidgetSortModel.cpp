#include "ProjectWidgetSortModel.h"
#include "ProjectWidget.h"

ProjectWidgetSortModel::ProjectWidgetSortModel(QObject* parent) :
        QSortFilterProxyModel(parent)
{

}

void ProjectWidgetSortModel::setFilteredContent(const QString& content)
{
    m_filteredContent = content;
    invalidateFilter();
}

bool ProjectWidgetSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (!left.parent().isValid() && !right.parent().isValid())
    {
        //top items (summaries) are sorted alphabethically
        QModelIndex leftSummaryIndex = left.sibling(left.row(), ProjectWidget::TAG_COLUMN);
        QString leftSummary = leftSummaryIndex.data().toString();
        QModelIndex rightSummaryIndex = right.sibling(right.row(), ProjectWidget::TAG_COLUMN);
        QString rightSummary = rightSummaryIndex.data().toString();
        return (leftSummary < rightSummary);
    }
    if (left.parent().isValid() && right.parent().isValid())
    {
        //tags stay unsorted
        QModelIndex leftTagIndex = left.sibling(left.row(), ProjectWidget::TAG_COLUMN);
        QModelIndex rightTagIndex = right.sibling(right.row(), ProjectWidget::TAG_COLUMN);
        return (leftTagIndex.data(Qt::UserRole).toInt() < rightTagIndex.data(Qt::UserRole).toInt());
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


bool ProjectWidgetSortModel::filterAcceptsColumn(int, const QModelIndex&) const
{
    return true;
}

bool ProjectWidgetSortModel::filterAcceptsRow(int source_row,
                                                const QModelIndex& source_parent) const
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
    QModelIndex vCardIndex = dataModel->index(source_row, ProjectWidget::TAG_COLUMN);
    for (int childRow = 0; childRow < dataModel->rowCount(vCardIndex); ++childRow)
    {
        QModelIndex contentIndex = dataModel->index(childRow, ProjectWidget::CONTENT_COLUMN, vCardIndex);
        QString content = contentIndex.data().toString();
        if (content == m_filteredContent)
        {
            return true;
        }
    }
    return false;
}
