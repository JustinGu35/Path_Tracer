#include "mainwindow.h"
#include <ui_mainwindow.h>
#include <cameracontrolshelp.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    multithread_stylesheet(), singlethread_stylesheet()
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    connect(ui->multithread_radioButton, SIGNAL(clicked()), ui->mygl, SLOT(slot_setMultithreaded()));
    connect(ui->singlethread_radioButton, SIGNAL(clicked()), ui->mygl, SLOT(slot_setSinglethreaded()));

    connect(ui->multithread_radioButton, SIGNAL(clicked()), this, SLOT(slot_multithreadStyle()));
    connect(ui->singlethread_radioButton, SIGNAL(clicked()), this, SLOT(slot_singlethreadStyle()));

    QFile f(":/styles/rainbow.stylesheet");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&f);
    multithread_stylesheet = in.readAll();
    f.close();

    QFile f2(":/styles/plain.stylesheet");
    f2.open(QFile::ReadOnly | QFile::Text);
    QTextStream in2(&f2);
    singlethread_stylesheet = in2.readAll();
    f2.close();

    slot_singlethreadStyle();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionLoad_Scene_triggered()
{
    ui->mygl->SceneLoadDialog();
}

void MainWindow::slot_ResizeToCamera(int w, int h)
{
    setFixedWidth(w);
    setFixedWidth(h);
}

void MainWindow::on_actionRender_triggered()
{
    ui->mygl->RenderScene();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    help.show();
}
void MainWindow::slot_DisableGUI(bool b)
{
    ui->integratorSpinBox->setEnabled(!b);
    ui->samplesSpinBox->setEnabled(!b);
    ui->recursionSpinBox->setEnabled(!b);
    ui->checkBox_Progressive->setEnabled(!b);
}


void MainWindow::slot_multithreadStyle() {
    this->setStyleSheet(multithread_stylesheet);
    this->update();
}

void MainWindow::slot_singlethreadStyle() {
    this->setStyleSheet(singlethread_stylesheet);
    this->update();
}
