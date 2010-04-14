#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class VCardProject;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private slots:
    void addNewProject();
    void openProject();
    void saveProject();
    void saveProjectAs();

//    void importVCards();
//    void exportVCards();

private:
    void showProject(VCardProject* project);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
