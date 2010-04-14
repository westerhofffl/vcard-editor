#ifndef PROJECTWIDGETITEMDELEGATE_H
#define PROJECTWIDGETITEMDELEGATE_H

#include <QItemDelegate>

class ProjectWidgetItemDelegate : public QItemDelegate
{
public:
    ProjectWidgetItemDelegate(QObject* parent);

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // PROJECTWIDGETITEMDELEGATE_H
