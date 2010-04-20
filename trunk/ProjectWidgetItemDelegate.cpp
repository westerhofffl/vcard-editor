#include "ProjectWidgetItemDelegate.h"

#include "ProjectWidget.h"

#include <QPainter>
#include <QPaintDevice>
#include <QPaintEngine>
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
        QModelIndex modifiedIndex = index.sibling(index.row(), ProjectWidget::TAG_COLUMN);

        QStyleOptionViewItemV4 modifiedOption(option);
        modifiedOption.rect.setX(option.decorationSize.width());
        int width = option.fontMetrics.width(modifiedIndex.data().toString())
                    + option.decorationSize.width();
        QPaintEngine* paintEngine = painter->paintEngine();
        if (paintEngine != 0)
        {
            QPaintDevice* paintDevice = paintEngine->paintDevice();
            width = paintDevice->width();
        }

        modifiedOption.rect.setWidth(width);
        QItemDelegate::paint(painter, modifiedOption, modifiedIndex);
        return;
    }

    QItemDelegate::paint(painter, option, index);
}
