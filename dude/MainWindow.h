#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class Project;
class QTreeWidgetItem;
class QTableWidgetItem;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();

private slots:
   void createNewProject();
   void startStopProject();
   void setProgressBar(int progress, QString text);
   void updateGroup(int index);

   void showTreePreview(QTreeWidgetItem* item);
   void showTablePreview(QTableWidgetItem* item);
   void checkTableItemState(QTableWidgetItem* item);

   void move();
   void unmove();
   void toggleMove();

private:
   void updateTable(int groupIndex, int fileIndex);
   QList<int> getSelectedFileList() const;

private:
    Ui::MainWindow *m_ui;
    Project* m_project;
};

#endif // MAINWINDOW_H
