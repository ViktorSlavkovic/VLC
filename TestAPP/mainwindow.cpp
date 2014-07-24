#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <string>
#include <cctype>

using std::vector;
using std::string;

///////////////////////////////////////////////////////////////////////////////////
//                      CONSTRUCTOR && DESTRUCTOR
///////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);   
    //Init timer
    T = new QTimer( this );
    connect( T, SIGNAL(timeout()), this, SLOT(on_T_Tick()) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////
//                      SLOTS
///////////////////////////////////////////////////////////////////////////////////

void MainWindow::on_btn_Plot_clicked()
{
    ui->gv_Input->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    ui->gv_Input->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    int h=ui->gv_Input->size().height();
    int w=ui->gv_Input->size().width();

    ui->gv_Decode->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    ui->gv_Decode->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    int hd=ui->gv_Decode->size().height();
    int wd=ui->gv_Decode->size().width();

    ui->gv_Result->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    ui->gv_Result->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    int hr=ui->gv_Result->size().height();
    int wr=ui->gv_Result->size().width();

    std::vector<double> v;
    loadData(v);
    if (v.empty()) return;
    ui->gv_Input->setScene(drawInputScene(w,h,v));
    tresholdMm(v);
    ui->gv_Decode->setScene(drawDecodeScene(wd,hd,v));
    string s;
    decode_it(v,s);
    ui->gv_Result->setScene(drawResultScene(wr,hr,v));
    ui->le_Res->setText(QString(s.c_str()));
}

void MainWindow::on_btn_Open_clicked()
{
    SP = new Serial(ui->tb_COM->text().toStdString().c_str());
    if (!SP->IsConnected()) {
        QMessageBox Msgbox;
        Msgbox.setText("Error");
        Msgbox.exec();
        delete SP;
        return;
    }

    T->start(100);

    ui->tb_COM->setEnabled(false);
    ui->btn_Open->setEnabled(false);
    ui->btn_Close->setEnabled(true);
    ui->btn_Send->setEnabled(true);
    ui->lineEdit->setEnabled(true);
    //ui->pte_Data->setEnabled(true);
}

void MainWindow::on_btn_Close_clicked()
{
    T->stop();
    delete SP;
    ui->tb_COM->setEnabled(true);
    ui->btn_Open->setEnabled(true);
    ui->btn_Close->setEnabled(false);
    ui->btn_Send->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    //ui->pte_Data->setEnabled(false);
}

void MainWindow::on_T_Tick() {
    char c[1];
    QString s;
    bool dec=false;
    int cnt=0;
    while(SP->ReadData(c,1)>=0) {
        if (c[0]=='.') {
            s.push_back(QChar(c[0]));
            dec=true;
        }
        else if (isdigit(c[0])) {
            if (cnt==2) {
                ui->pte_Data->appendPlainText(s);
                cnt=0;
                s.clear();
                dec=false;
            }
            else if(dec) cnt++;
            s.push_back(QChar(c[0]));
        }
    }
}

void MainWindow::on_btn_Send_clicked()
{
    std::string s=ui->lineEdit->text().toStdString();
    if (s.empty()) return;
    SP->WriteData(s.c_str(),s.size());
}

///////////////////////////////////////////////////////////////////////////////////
//                      DATA MANIP
///////////////////////////////////////////////////////////////////////////////////

void MainWindow::loadData(std::vector<double> & v) {
    v.clear();
    QString s_temp = ui->pte_Data->toPlainText();
    QStringList lines = s_temp.split("\n");
    for(QStringList::iterator it=lines.begin(); it!=lines.end(); it++)
        v.push_back(it->toDouble());
    if (v.size()==1) v.pop_back();
}

void MainWindow::tresholdMm(vector<double> & v) {
    double M = *max_element(v.begin(), v.end());
    double m = *min_element(v.begin(), v.end());

    double delta = M - m;

    double tresh_M = M - delta / 2;

    for (vector<double>::iterator it=v.begin(); it!=v.end(); it++)
        if (*it > tresh_M) *it=1;
        //else if (*it < tresh_m) *it=0;
        else *it=0;
}

void MainWindow::decode_it(vector<double> & v, string & s) {
    vector<int> p;

    int avg;
    {
        int total=0;
        int num=0;
        bool check=false;

        for (vector<double>::iterator it=v.begin(); it!=v.end(); it++) {
            if (*it==1) {
                check=true;
                num++;
            }
            if (check && *it<1) {
                check=false;
                p.push_back(num);
                total+=num;
                num=0;
            }
        }

        avg=total/p.size();
    }

    //vector<int> plot;
    v.clear();

    int bc=0;
    int c=0;
    for (vector<int>::iterator it=p.begin(); it!=p.end(); it++) {
        if (bc==8) {
            bc=0;
            s.push_back(c);
            c=0;
        }

        c*=2;
        if (*it > avg) {
            c+=1;
            v.push_back(1);
        }
        else v.push_back(0);

        bc++;
    }

    if (bc==8) {
        bc=0;
        s.push_back(c);
        c=0;
    }

    //v=plot;
}

///////////////////////////////////////////////////////////////////////////////////
//                      SCENE DTAWING
///////////////////////////////////////////////////////////////////////////////////

QGraphicsScene* MainWindow::drawResultScene(int w, int h, std::vector<double> & v) {
    using std::vector;

    double scW=w/(v.size()-1);

    QGraphicsScene * scene = new QGraphicsScene(0,0,w,h);

    int px=0;
    int py=0;
    for(vector<double>::size_type i=0; i<v.size(); i++) {
        int x = (int)(i*scW);
        int y = h-((v[i]+1)*h/3);
        scene->addLine(px,py,x,y,QPen(QColor(0,0,255)));
        int a=(x+px)/2;
        scene->addLine(a,0,a,h,QPen(QColor(0,0,0)));
        px=x;
        py=y;
    };
    return scene;
}

QGraphicsScene* MainWindow::drawDecodeScene(int w, int h, std::vector<double> & v) {
    using std::vector;

    double scW=w/(v.size()-1);

    QGraphicsScene * scene = new QGraphicsScene(0,0,w,h);

    int px=0;
    int py=h-((v[0]+1)*h/3);
    for(vector<double>::size_type i=0; i<v.size(); i++) {
        int x = (int)(i*scW);
        int y = h-((v[i]+1)*h/3);
        scene->addLine(px,py,x,y,QPen(QColor(0,255,0)));
        px=x;
        py=y;
    };
    return scene;
}

QGraphicsScene* MainWindow::drawInputScene(int w, int h, std::vector<double> & v) {
    using std::vector;

    double M=*std::max_element(v.begin(), v.end());
    double m=*std::min_element(v.begin(), v.end());

    double scW=w/(v.size()-1);
    double scH=h/(M-m);

    QGraphicsScene * scene = new QGraphicsScene(0,0,w,h);


    int px=0;
    int py=h-(int)((v[0]-m)*scH);
    for(vector<double>::size_type i=0; i<v.size(); i++) {
        int x = (int)(i*scW);
        int y = h-(int)((v[i]-m)*scH);
        scene->addLine(px,py,x,y,QPen(QColor(255,0,0)));
        px=x;
        py=y;
    };
    return scene;
}

