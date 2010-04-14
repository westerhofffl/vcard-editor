#ifndef PROJECTWIDGETSORTMODEL_H
#define PROJECTWIDGETSORTMODEL_H

#include <QSortFilterProxyModel>

class ProjectWidgetSortModel : public QSortFilterProxyModel
{
public:
    ProjectWidgetSortModel(QObject* parent);

    void setFilteredContent(const QString& content);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    bool filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const;
    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
private:
    QString m_filteredContent;
};

#endif // PROJECTWIDGETSORTMODEL_H
