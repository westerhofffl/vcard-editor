#include "ProjectWidgetItemDelegate.h"

#include "ProjectWidget.h"

#include <QStyleOptionViewItemV4>

ProjectWidgetItemDelegate::ProjectWidgetItemDelegate(QObject *parent) :
        QItemDelegate(parent)
{
}

void ProjectWidgetItemDelegate::paint ( QPainter * painter,
                                        const QStyleOptionViewItem & option,
                                        const QModelIndex & index ) const
{

    if (index.isValid() && !index.parent().isValid())
    {
        QStyleOptionViewItemV4 modifiedOption(option);
        modifiedOption.rect.setX(option.decorationSize.width());
        modifiedOption.rect.setWidth(1000);
        QModelIndex modifiedIndex = index.sibling(index.row(), ProjectWidget::TAG_COLUMN);
        QItemDelegate::paint(painter, modifiedOption, modifiedIndex);
        return;
    }

    QItemDelegate::paint(painter, option, index);
}
