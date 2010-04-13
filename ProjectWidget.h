#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

namespace Ui {
    class ProjectWidget;
}

class VCardProject;
class VCard;

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
   void updateVCard(int row,
                    const VCard& vCard);
   void updateTag(int row,
                  const QString& tag,
                  const QString& content);
   int getTagColumn(const QString& tag);

private:
   Ui::ProjectWidget* m_ui;
   VCardProject* m_project;
};

#endif // PROJECTWIDGET_H
