#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

namespace Ui {
    class ProjectWidget;
}

class VCardProject;

#include <QWidget>


class ProjectWidget : public QWidget
{
public:
   ProjectWidget(VCardProject* project, QWidget *parent);

   ~ProjectWidget();

   const VCardProject& getProject() const;

protected:
   void changeEvent(QEvent *e);

private:
   Ui::ProjectWidget* m_ui;
   VCardProject* m_project;
};

#endif // PROJECTWIDGET_H
