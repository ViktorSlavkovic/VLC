#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QTimer>

#include <vector>

#include "Serial.h"

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
        void on_btn_Plot_clicked();

        void on_btn_Open_clicked();

        void on_btn_Close_clicked();

        void on_T_Tick();

        void on_btn_Send_clicked();

    private:
        Ui::MainWindow *ui;

        /////////////////////////////////////

        QTimer * T;
        Serial * SP;

        QGraphicsScene * drawInputScene(int w, int h, std::vector<double> & v);
        QGraphicsScene * drawDecodeScene(int w, int h, std::vector<double> & v);
        QGraphicsScene * drawResultScene(int w, int h, std::vector<double> & v);

        void loadData(std::vector<double> & v);
        void tresholdMm(std::vector<double> & v);
        void decode_it(std::vector<double> & v, std::string & s);
};

#endif // MAINWINDOW_H
