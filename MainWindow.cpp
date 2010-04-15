#include "MainWindow.h"
#include "ProjectWidget.h"
#include "ui_MainWindow.h"
#include "VCardProject.h"
#include <QFileDialog>
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

    QString fileName =
          //"/home/gogi/Downloads/pcsc_pcsc_00001.vcf";
          "G:\\pcsc_pcsc_00001.vcf";
    QFile file(fileName);
    //showProject(new VCardProject(file));
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

void MainWindow::updateProjectState()
{
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    ProjectWidget* currentProjectWidget = dynamic_cast<ProjectWidget*>(currentWidget);
    if (currentProjectWidget == 0)
    {
        ui->actionUndo->setEnabled(false);
        ui->actionRedo->setEnabled(false);
        return;
    }

    ui->actionUndo->setEnabled(currentProjectWidget->canUndo());
    ui->actionRedo->setEnabled(currentProjectWidget->canRedo());

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
