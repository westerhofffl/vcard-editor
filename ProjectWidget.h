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
    Q_OBJECT

public:
   ProjectWidget(VCardProject* project, QWidget *parent);

   ~ProjectWidget();

   const VCardProject& getProject() const;

   void updateProjectView();

protected:
   void changeEvent(QEvent *e);

private slots:
    void on_hideDuplicatesButton_clicked();
    void on_showDuplicatesButton_clicked();
    void on_removeTagButton_clicked();
    void on_insertTagButton_clicked();
    void on_removeVCardButton_clicked();
    void on_addVCardButton_clicked();
    void on_expandButton_clicked();

    void updateTagData();
    void updateButtons();
    void showSelectedDuplicates();

private:
    int getVCardId(const QModelIndex& index) const;
    int getTagIndex(const QModelIndex& index) const;

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

   class SortModel;
   SortModel* m_model;

   ///

   class SortModel : public QSortFilterProxyModel
   {
   public:
       SortModel(QObject* parent);

       void setFilteredContent(const QString& content);

   protected:
       bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

       bool filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const;
       bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
   private:
       QString m_filteredContent;
   };

};

#endif // PROJECTWIDGET_H
