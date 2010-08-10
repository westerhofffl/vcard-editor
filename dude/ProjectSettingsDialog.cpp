#include "ProjectSettingsDialog.h"
#include "ui_ProjectSettingsDialog.h"

#include <QFileDialog>

ProjectSettingsDialog::ProjectSettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProjectSettingsDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->folderLineEdit, SIGNAL(textChanged(QString)), SLOT(setDefaultDuplicatesFolder()));
    connect(m_ui->folderToolButton, SIGNAL(clicked()), SLOT(chooseFolder()));

    connect(m_ui->duplicatesToolButton, SIGNAL(clicked()), SLOT(chooseDuplicatesFolder()));
    connect(m_ui->duplicatesDefaultCheckBox, SIGNAL(clicked()), SLOT(setDefaultDuplicatesFolder()));
    connect(m_ui->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));

    setDefaultDuplicatesFolder();
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete m_ui;
}

QString ProjectSettingsDialog::getFolderName() const
{
   return m_ui->folderLineEdit->text();
}

QString ProjectSettingsDialog::getDuplicatesFolderName() const
{
   return m_ui->duplicatesLineEdit->text();
}

void ProjectSettingsDialog::chooseFolder()
{
   QString folderName = QFileDialog::getExistingDirectory(this, "Choose a folder");
   if (!folderName.isEmpty())
   {
      m_ui->folderLineEdit->setText(folderName);
   }
}

void ProjectSettingsDialog::chooseDuplicatesFolder()
{
   QString folderName = QFileDialog::getExistingDirectory(this, "Choose a folder");
   if (!folderName.isEmpty())
   {
      m_ui->duplicatesLineEdit->setText(folderName);
   }
}

void ProjectSettingsDialog::setDefaultDuplicatesFolder()
{
   if (m_ui->duplicatesDefaultCheckBox->isChecked())
   {
      m_ui->duplicatesLineEdit->setText(m_ui->folderLineEdit->text().append("_dup"));
   }
}

bool ProjectSettingsDialog::parseSubfolders() const
{
   return m_ui->subfolderCheckBox->isChecked();
}
