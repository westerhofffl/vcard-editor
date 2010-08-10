#ifndef PROJECTSETTINGSDIALOG_H
#define PROJECTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
    class ProjectSettingsDialog;
}

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSettingsDialog(QWidget *parent = 0);
    ~ProjectSettingsDialog();

    QString getFolderName() const;
    bool parseSubfolders() const;

private slots:
   void chooseFolder();

private:
    Ui::ProjectSettingsDialog *m_ui;
};

#endif // PROJECTSETTINGSDIALOG_H
