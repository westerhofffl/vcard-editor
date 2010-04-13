#ifndef NEWVCARDDIALOG_H
#define NEWVCARDDIALOG_H

#include <QDialog>

namespace Ui {
    class NewVCardDialog;
}

class NewVCardDialog : public QDialog {
    Q_OBJECT
public:
    NewVCardDialog(QWidget *parent = 0);
    ~NewVCardDialog();

    QString getFullName();
    QString getName() const;

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_nameLineEdit_textEdited(QString );
    void on_fullNameLineEdit_textEdited(QString );
    void checkButtons();

private:
    Ui::NewVCardDialog *m_ui;

};

#endif // NEWVCARDDIALOG_H
