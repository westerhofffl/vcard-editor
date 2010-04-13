#include "ProjectWidget.h"
#include "ui_ProjectWidget.h"
#include "VCardProject.h"

ProjectWidget::ProjectWidget(VCardProject* project, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ProjectWidget),
    m_project(project)
{
    m_ui->setupUi(this);
}

ProjectWidget::~ProjectWidget()
{
    delete m_ui;
}

const VCardProject& ProjectWidget::getProject() const
{
   return *m_project;
}

void ProjectWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
