#include "MainWindow.h"
#include "ProjectWidget.h"
#include "ui_MainWindow.h"
#include "VCardProject.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_project, SIGNAL(triggered()), SLOT(addNewProject()));
    connect(ui->actionOpen_project, SIGNAL(triggered()), SLOT(openProject()));
    connect(ui->actionSave_project, SIGNAL(triggered()), SLOT(saveProject()));
    connect(ui->actionSave_project_as, SIGNAL(triggered()), SLOT(saveProjectAs()));

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
    project.saveTo(file);
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
       const VCardProject& project = currentProjectWidget->getProject();
       QFile file(fileName);
       project.saveTo(file);
    }
}

void MainWindow::showProject(VCardProject* project)
{
   ProjectWidget* projectWidget = new ProjectWidget(project, this);
   ui->tabWidget->addTab(projectWidget, project->getFileName());
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
