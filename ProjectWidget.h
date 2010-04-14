#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

namespace Ui {
    class ProjectWidget;
}

class ProjectWidgetSortModel;
class VCardProject;
class VCard;

class QModelIndex;
#include <QWidget>

class ProjectWidget : public QWidget
{
    Q_OBJECT

public:
   ProjectWidget(VCardProject* project, QWidget *parent);

   ~ProjectWidget();

   const VCardProject& getProject() const;

   void updateProjectView();

   enum Columns
   {
       TAG_COLUMN,
       PROPERTIES_COLUMN,
       CONTENT_COLUMN,
       COLUMN_COUNT
   };

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
    void updateTagInfo();
    void showSelectedDuplicates();

private:
    int getVCardId(const QModelIndex& index) const;
    int getTagIndex(const QModelIndex& index) const;

private:
   Ui::ProjectWidget* m_ui;
   VCardProject* m_project;

   ProjectWidgetSortModel* m_model;

};

#endif // PROJECTWIDGET_H
