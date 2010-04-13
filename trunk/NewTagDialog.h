#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H

#include <QDialog>

namespace Ui {
    class NewTagDialog;
}

class NewTagDialog : public QDialog {
    Q_OBJECT
public:
    NewTagDialog(QWidget *parent = 0);
    ~NewTagDialog();

    QString getName() const;
    QString getProperties() const;
    QString getContent() const;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::NewTagDialog *m_ui;

private slots:
    void on_contentTextEdit_textChanged();
    void on_propertiesLineEdit_textEdited(QString );
    void on_nameLineEdit_textEdited(QString );
    void on_cancelButton_clicked();
    void on_okButton_clicked();
    void updateButtons();
};

#endif // NEWTAGDIALOG_H
