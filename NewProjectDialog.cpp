#include "NewProjectDialog.h"
#include "ui_NewProjectDialog.h"

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    ui->versionComboBox->addItem(VCardProject::getVersionAsString(VCardProject::VER_2_1), VCardProject::VER_2_1);
    ui->versionComboBox->addItem(VCardProject::getVersionAsString(VCardProject::VER_3_0), VCardProject::VER_3_0);
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

QString NewProjectDialog::getName() const
{
    return ui->nameLineEdit->text();
}
VCardProject::Version NewProjectDialog::getVersion() const
{
    int currentIndex = ui->versionComboBox->currentIndex();
    int intValue = ui->versionComboBox->itemData(currentIndex).toInt();
    VCardProject::Version value = static_cast<VCardProject::Version>(intValue);
    return value;
}
bool NewProjectDialog::allowMultipleTags() const
{
    return ui->allowMultipleTagsCheckBox->isChecked();
}

void NewProjectDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
