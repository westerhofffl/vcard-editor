#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include "VCardProject.h"
#include <QDialog>
#include <QFile>

namespace Ui {
    class NewProjectDialog;
}

class NewProjectDialog : public QDialog {
    Q_OBJECT
public:
    NewProjectDialog(QWidget *parent = 0);
    ~NewProjectDialog();

    QFile getTargetFileName() const;
    VCardProject::Version getVersion() const;
    bool allowMultipleTags() const;    

protected:
    void changeEvent(QEvent *e);

private:
    Ui::NewProjectDialog *ui;
};

#endif // NEWPROJECTDIALOG_H
