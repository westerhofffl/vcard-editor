#include "NewVCardDialog.h"
#include "ui_NewVCardDialog.h"

NewVCardDialog::NewVCardDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::NewVCardDialog)
{
    m_ui->setupUi(this);
}

QString NewVCardDialog::getFullName()
{
    return m_ui->fullNameLineEdit->text();
}

QString NewVCardDialog::getName() const
{
    return m_ui->nameLineEdit->text();
}

NewVCardDialog::~NewVCardDialog()
{
    delete m_ui;
}

void NewVCardDialog::changeEvent(QEvent *e)
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

void NewVCardDialog::on_fullNameLineEdit_textEdited(QString )
{
    checkButtons();
}

void NewVCardDialog::on_nameLineEdit_textEdited(QString )
{
    checkButtons();
}

void NewVCardDialog::checkButtons()
{
    m_ui->okButton->setEnabled(!getFullName().isEmpty() || !getName().isEmpty());
}


void NewVCardDialog::on_cancelButton_clicked()
{
    reject();
}

void NewVCardDialog::on_okButton_clicked()
{
    accept();
}
