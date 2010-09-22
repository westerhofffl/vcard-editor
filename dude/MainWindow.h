#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class Project;
class QButtonGroup;
class QPushButton;
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

   void updateFilter();
   void updateFilter(QTreeWidgetItem* item);

   void showTreePreview(QTreeWidgetItem* item);
   void showExternTreePreview();
   void showTablePreview();
   void showExternTablePreview();
   void checkTableItemState(QTableWidgetItem* item);

   void move();
   void unmove();
   void toggleMove();

private:
   void updateTable(int groupIndex, int fileIndex);
   QList<int> getSelectedFileList() const;

   void showPreview(int fileIndex,
                    QPushButton* button);

   enum
   {
      TREE_ROLE_FILE_INDEX = Qt::UserRole,
      TREE_ROLE_GROUP_INDEX,
      TREE_ROLE_FILE_COUNT,
      TREE_ROLE_MOVED_COUNT
   };

   enum
   {
      SHOW_ALL,
      SHOW_DUPLICATED_ONLY,
      SHOW_NOT_RESOLVED_ONLY,
      SHOW_REMOVED_ONLY
   };

private:
    Ui::MainWindow *m_ui;
    Project* m_project;
    QButtonGroup* m_showButtonGroup;

    int m_tableGroupIndex;
    int m_tableFileIndex;
};

#endif // MAINWINDOW_H
