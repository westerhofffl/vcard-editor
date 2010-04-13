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
