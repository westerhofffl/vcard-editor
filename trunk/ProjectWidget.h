#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

namespace Ui {
    class ProjectWidget;
}

class VCardProject;
class VCard;
#include <QSortFilterProxyModel>
#include <QWidget>


class ProjectWidget : public QWidget
{
public:
   ProjectWidget(VCardProject* project, QWidget *parent);

   ~ProjectWidget();

   const VCardProject& getProject() const;

   void updateProjectView();

protected:
   void changeEvent(QEvent *e);

private:

private:
   Ui::ProjectWidget* m_ui;
   VCardProject* m_project;

   enum Columns
   {
       TAG_COLUMN,
       PROPERTIES_COLUMN,
       CONTENT_COLUMN,
       COLUMN_COUNT
   };

   QSortFilterProxyModel* m_model;

   ///

   class SortModel : public QSortFilterProxyModel
   {
   public:
       SortModel(QObject* parent);

   protected:
       bool lessThan(const QModelIndex &left, const QModelIndex &right);


   };
};

#endif // PROJECTWIDGET_H
