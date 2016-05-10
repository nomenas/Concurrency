#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filesearchengine.h"

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
    void on_pushButton_clicked();
    void on_btnCancel_clicked();

private:
    Ui::MainWindow *ui;
    QScopedPointer<FileSearchEngine> m_searchEngine;
};

#endif // MAINWINDOW_H
