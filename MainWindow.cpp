#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NewProjectDialog.h"

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
    NewProjectDialog newProjectDialog(this);
    if (newProjectDialog.exec() == QDialog::Accepted)
    {

    }
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
