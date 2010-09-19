#include "ProjectSettingsDialog.h"
#include "ui_ProjectSettingsDialog.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

ProjectSettingsDialog::ProjectSettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProjectSettingsDialog)
{
    m_ui->setupUi(this);

    setWindowTitle("Settings");

    connect(m_ui->folderLineEdit, SIGNAL(textChanged(QString)), SLOT(setDefaultDuplicatesFolder()));
    connect(m_ui->folderToolButton, SIGNAL(clicked()), SLOT(chooseFolder()));

    connect(m_ui->duplicatesToolButton, SIGNAL(clicked()), SLOT(chooseDuplicatesFolder()));
    connect(m_ui->duplicatesDefaultCheckBox, SIGNAL(clicked()), SLOT(setDefaultDuplicatesFolder()));
    connect(m_ui->buttonBox, SIGNAL(accepted()), SLOT(checkSettings()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));

    m_ui->folderLineEdit->setText(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
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
   QString folderName = QFileDialog::getExistingDirectory(this, "Choose a folder", getFolderName());
   if (!folderName.isEmpty())
   {
      m_ui->folderLineEdit->setText(QDir::toNativeSeparators(folderName));
   }
}

void ProjectSettingsDialog::chooseDuplicatesFolder()
{
   QString folderName = QFileDialog::getExistingDirectory(this, "Choose a folder", getDuplicatesFolderName());
   if (!folderName.isEmpty())
   {
      m_ui->duplicatesLineEdit->setText(QDir::toNativeSeparators(folderName));
   }
}

void ProjectSettingsDialog::setDefaultDuplicatesFolder()
{
   if (m_ui->duplicatesDefaultCheckBox->isChecked())
   {
      QString folderName = QDir::toNativeSeparators(getFolderName());
      if (folderName.endsWith(QDir::separator()))
      {
         folderName = folderName.left(folderName.length() - 1);
      }

      m_ui->duplicatesLineEdit->setText(folderName.append("_dup"));
   }
   m_ui->duplicatesLineEdit->setEnabled(!m_ui->duplicatesDefaultCheckBox->isChecked());
   m_ui->duplicatesToolButton->setEnabled(!m_ui->duplicatesDefaultCheckBox->isChecked());
}

bool ProjectSettingsDialog::parseSubfolders() const
{
   return m_ui->subfolderCheckBox->isChecked();
}

void ProjectSettingsDialog::checkSettings()
{
   QDir sourceDir(getFolderName());
   QString errorMessage;
   if (!sourceDir.exists())
   {
      errorMessage = QString("Directory '%1' doesn't exists!").arg(getFolderName());
   }
   QDir removedDir(getDuplicatesFolderName());
   if (sourceDir == removedDir)
   {
      errorMessage = QString("The folder and the duplicates folder are the same!");
   }
   while(removedDir.cdUp())
   {
      if (sourceDir == removedDir)
      {
         errorMessage = QString("The duplicates folder is a subfolder of the original one!");
      }
   }
   removedDir = getDuplicatesFolderName();
   while(sourceDir.cdUp())
   {
      if (sourceDir == removedDir)
      {
         errorMessage = QString("The original folder is a subfolder of the duplicates one!");
      }
   }
   if (errorMessage.isEmpty())
   {
      accept();
   }
   else
   {
      QMessageBox::warning(this, "Invalid setting", errorMessage);
   }
}
