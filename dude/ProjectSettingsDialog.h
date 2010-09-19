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
    QString getDuplicatesFolderName() const;
    bool parseSubfolders() const;

private slots:
   void chooseFolder();
   void chooseDuplicatesFolder();
   void setDefaultDuplicatesFolder();
   void checkSettings();

private:
    Ui::ProjectSettingsDialog *m_ui;
};

#endif // PROJECTSETTINGSDIALOG_H
