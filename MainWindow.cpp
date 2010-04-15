#include "MainWindow.h"
#include "ProjectWidget.h"
#include "ui_MainWindow.h"
#include "VCard.h"
#include "VCardProject.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_project, SIGNAL(triggered()), SLOT(addNewProject()));
    connect(ui->actionOpen_project, SIGNAL(triggered()), SLOT(openProject()));
    connect(ui->actionSave_project, SIGNAL(triggered()), SLOT(saveProject()));
    connect(ui->actionSave_project_as, SIGNAL(triggered()), SLOT(saveProjectAs()));
    connect(ui->actionClose_project, SIGNAL(triggered()), SLOT(closeProject()));

    connect(ui->actionUndo, SIGNAL(triggered()), SLOT(undo()));
    connect(ui->actionRedo, SIGNAL(triggered()), SLOT(redo()));

    connect(ui->actionImport, SIGNAL(triggered()), SLOT(importProject()));
    connect(ui->actionExport, SIGNAL(triggered()), SLOT(exportProject()));

    QString fileName =
          //"/home/gogi/Downloads/pcsc_pcsc_00001.vcf";
          "G:\\pcsc_pcsc_00001.vcf";
    QFile file(fileName);
    //showProject(new VCardProject(file));

    updateProjectState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addNewProject()
{
   QString fileName =
         QFileDialog::getSaveFileName(this, "Choose the target location", "", "*.vcf");
   if (!fileName.isEmpty())
   {
      QFile file(fileName);
      showProject(new VCardProject(file));
   }
   updateProjectState();
}

void MainWindow::openProject()
{
   QString fileName =
         QFileDialog::getOpenFileName(this, "Choose the target location", "", "*.vcf");
   if (!fileName.isEmpty())
   {
      QFile file(fileName);
      showProject(new VCardProject(file));
   }
   updateProjectState();
}

void MainWindow::saveProject()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }
    const VCardProject& project = currentProjectWidget->getProject();
    QFile file(project.getFileName());
    currentProjectWidget->saveProject(file);

    int currentTabIndex = ui->tabWidget->currentIndex();
    QString tabName = project.getFileName();
    if (currentProjectWidget->isProjectModified())
    {
        tabName.append(" *");
    }
    ui->tabWidget->setTabText(currentTabIndex, tabName);
}

void MainWindow::saveProjectAs()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }
    QString fileName =
          QFileDialog::getSaveFileName(this, "Choose the target location", "", "*.vcf");
    if (!fileName.isEmpty())
    {
       QFile file(fileName);
       currentProjectWidget->saveProject(file);
       const VCardProject& project = currentProjectWidget->getProject();

       int currentTabIndex = ui->tabWidget->currentIndex();
       QString tabName = project.getFileName();
       if (currentProjectWidget->isProjectModified())
       {
           tabName.append(" *");
       }
       ui->tabWidget->setTabText(currentTabIndex, tabName);
   }
}

void MainWindow::closeProject()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }
    if (currentProjectWidget->isProjectModified())
    {
        if (QMessageBox::warning(this, "Warning",
            QString("The project has been changed. Save?"),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            saveProject();
        }
    }
    delete currentProjectWidget;
    updateProjectState();
}

void MainWindow::undo()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }
    currentProjectWidget->undo();
}

void MainWindow::redo()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }
    currentProjectWidget->redo();
}

void MainWindow::importProject()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }

    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        "Select files to import", "", "*.vcf");
    if (fileNames.isEmpty())
    {
        return;
    }
    VCardProject project(currentProjectWidget->getProject());
    foreach(QString fileName, fileNames)
    {
        QFile file(fileName);
        VCardProject importedProject(file);
        foreach(int vCardId, importedProject.getVCardIdList())
        {
            project.addVCard(importedProject.getVCard(vCardId));
        }
    }
    currentProjectWidget->setProject(project, false);
}

void MainWindow::exportProject()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        return;
    }

    QString targetDir = QFileDialog::getExistingDirectory(this,
        "Select target directory");
    if (targetDir.isEmpty())
    {
        return;
    }

    const VCardProject& project = currentProjectWidget->getProject();
    foreach(int vCardId, project.getVCardIdList())
    {
        VCard vCard = project.getVCard(vCardId);
        QList<int> tagIndexList = vCard.getTagIndexList("FN");
        if (tagIndexList.isEmpty())
        {
            tagIndexList += vCard.getTagIndexList("N");
        }
        QStringList targetFileNameList;
        foreach(int tagIndex, tagIndexList)
        {
            QString content = vCard.getTagContent(tagIndex);
            content.replace(";", " ");
            targetFileNameList.append(content);
        }
        QString targetFileName = targetFileNameList.join(" ");
        int fileNameIndex = 1;
        QFileInfo fileInfo(QDir(targetDir), targetFileName);
        while (fileInfo.exists())
        {
            fileInfo = QFileInfo(QDir(targetDir), QString("%1_%2")
                                 .arg(targetFileName).arg(fileNameIndex));
            ++fileNameIndex;
        }
        VCardProject singleProject(QFile(fileInfo.absoluteFilePath()));
        singleProject.addVCard(vCard);
        singleProject.saveTo(QFile(fileInfo.absoluteFilePath()));
    }
}

void MainWindow::updateProjectState()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        ui->actionUndo->setEnabled(false);
        ui->actionRedo->setEnabled(false);
        ui->actionImport->setEnabled(false);
        ui->actionExport->setEnabled(false);
        return;
    }

    ui->actionUndo->setEnabled(currentProjectWidget->canUndo());
    ui->actionRedo->setEnabled(currentProjectWidget->canRedo());
    ui->actionImport->setEnabled(true);
    ui->actionExport->setEnabled(true);

    const VCardProject& project = currentProjectWidget->getProject();
    int currentTabIndex = ui->tabWidget->currentIndex();
    QString tabName = project.getFileName();
    if (currentProjectWidget->isProjectModified())
    {
        tabName.append(" *");
    }
    ui->tabWidget->setTabText(currentTabIndex, tabName);
}

void MainWindow::showProject(VCardProject* project)
{
   ProjectWidget* projectWidget = new ProjectWidget(project, this);
   ui->tabWidget->addTab(projectWidget, project->getFileName());

   connect(projectWidget, SIGNAL(projectChanged()), SLOT(updateProjectState()));
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
