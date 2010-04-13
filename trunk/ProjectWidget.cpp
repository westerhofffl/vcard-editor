#include "ProjectWidget.h"
#include "ui_ProjectWidget.h"
#include "VCard.h"
#include "VCardProject.h"

ProjectWidget::ProjectWidget(VCardProject* project, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ProjectWidget),
    m_project(project)
{
    m_ui->setupUi(this);

    m_ui->tableWidget->setSortingEnabled(true);
    m_ui->tableWidget->clear();
    m_ui->tableWidget->setRowCount(project->getVCardCount());
    for(int index = 0; index < project->getVCardCount(); ++index)
    {
       VCard vCard = project->getVCard(index);
       updateVCard(index, vCard);
    }
    m_ui->tableWidget->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
}

ProjectWidget::~ProjectWidget()
{
    delete m_ui;
}

const VCardProject& ProjectWidget::getProject() const
{
   return *m_project;
}

void ProjectWidget::updateVCard(int row, const VCard& vCard)
{
   if (row >= m_ui->tableWidget->rowCount())
   {
      m_ui->tableWidget->setRowCount(row + 1);
   }
   for (int col = 0; col < m_ui->tableWidget->columnCount(); ++col)
   {
      QTableWidgetItem* item = new QTableWidgetItem();
      m_ui->tableWidget->setItem(row, col, item);
   }
   for (int index = 0; index < vCard.getTagCount(); ++index)
   {
      QString tag = vCard.getTag(index);
      QString content = vCard.getTagContent(index);
      updateTag(row, tag, content);
   }
}

void ProjectWidget::updateTag(int row, const QString& tag, const QString& content)
{
   int col = getTagColumn(tag);
   QTableWidgetItem* item = m_ui->tableWidget->item(row, col);
   if (item == 0)
   {
      item = new QTableWidgetItem();
      m_ui->tableWidget->setItem(row, col, item);
   }
   QString text = item->text();
   if (!text.isEmpty())
   {
      text.append("; ");
   }
   text.append(content);
   item->setText(text);
}

int ProjectWidget::getTagColumn(const QString& tag)
{
   for(int col = 0; col < m_ui->tableWidget->columnCount(); ++col)
   {
      QTableWidgetItem* headItem = m_ui->tableWidget->horizontalHeaderItem(col);
      if (headItem->text() == tag)
      {
         return col;
      }
   }
   int col = m_ui->tableWidget->columnCount();
   m_ui->tableWidget->setColumnCount(col + 1);
   m_ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(tag));
   if ((QString::compare("BEGIN", tag, Qt::CaseInsensitive) == 0) ||
       (QString::compare("END", tag, Qt::CaseInsensitive) == 0))
   {
      m_ui->tableWidget->hideColumn(col);
   }
   return col;
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

