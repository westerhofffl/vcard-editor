#include "ProjectSettingsDialog.h"
#include "ui_ProjectSettingsDialog.h"

#include <QFileDialog>

ProjectSettingsDialog::ProjectSettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProjectSettingsDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->folderToolButton, SIGNAL(clicked()), SLOT(chooseFolder()));
    connect(m_ui->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete m_ui;
}

QString ProjectSettingsDialog::getFolderName() const
{
   return m_ui->folderLineEdit->text();
}

void ProjectSettingsDialog::chooseFolder()
{
   QString folderName = QFileDialog::getExistingDirectory(this, "Choose a folder");
   if (!folderName.isEmpty())
   {
      m_ui->folderLineEdit->setText(folderName);
   }
}


bool ProjectSettingsDialog::parseSubfolders() const
{
   return m_ui->subfolderCheckBox->isChecked();
}
