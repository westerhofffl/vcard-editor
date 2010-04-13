#include "NewTagDialog.h"
#include "ui_NewTagDialog.h"

NewTagDialog::NewTagDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::NewTagDialog)
{
    m_ui->setupUi(this);
}

NewTagDialog::~NewTagDialog()
{
    delete m_ui;
}

QString NewTagDialog::getName() const
{
    return m_ui->nameLineEdit->text();
}
QString NewTagDialog::getProperties() const
{
    return m_ui->propertiesLineEdit->text();
}
QString NewTagDialog::getContent() const
{
    return m_ui->contentTextEdit->toPlainText();
}

void NewTagDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void NewTagDialog::on_okButton_clicked()
{
    accept();
}

void NewTagDialog::on_cancelButton_clicked()
{
    reject();
}

void NewTagDialog::on_nameLineEdit_textEdited(QString )
{
    updateButtons();
}

void NewTagDialog::on_propertiesLineEdit_textEdited(QString )
{
    updateButtons();
}

void NewTagDialog::on_contentTextEdit_textChanged()
{
    updateButtons();
}

void NewTagDialog::updateButtons()
{
    m_ui->okButton->setEnabled(!getName().isEmpty());
}
