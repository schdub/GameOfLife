#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"

#include <QLabel>
#include <QSpinBox>
#include <QStatusBar>
#include <QComboBox>
#include <QMap>
#include <limits>

typedef QList<QPoint> PPoints;
static QMap<QString, PPoints> sPresets;

void MainWindow::initPresets() {
    sPresets.clear();
    sPresets["Empty"]    = { };
    sPresets["Glider"]   = { {1,0}, {2,1}, {2,2}, {1,2}, {0,2} };
    sPresets["Exploder"] = { {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {2,0}, {2,4}, {4,0}, {4,1}, {4,2}, {4,3}, {4,4} };
    sPresets["Tumbler"]  = { {0,3}, {0,4}, {0,5}, {1,0}, {1,1}, {1,5}, {2,0}, {2,1}, {2,2}, {2,3}, {2,4}, {4,0}, {4,1}, {4,2}, {4,3}, {4,4}, {5,0}, {5,1}, {5,5}, {6,3}, {6,4}, {6,5} };
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initPresets();

    ui->actionStart->setVisible(true);
    ui->actionNext->setVisible(true);
    ui->actionPause->setVisible(false);
    connect(ui->actionStart, SIGNAL(triggered(bool)), ui->centralWidget, SLOT(start()));
    connect(ui->actionNext, SIGNAL(triggered(bool)), ui->centralWidget, SLOT(generationUpdate()));
    connect(ui->actionPause, SIGNAL(triggered(bool)), ui->centralWidget, SLOT(pause()));
    connect(ui->actionShow_grid, SIGNAL(triggered(bool)), ui->centralWidget, SLOT(setDrawGrid(bool)));
    connect(ui->centralWidget, SIGNAL(started()), this, SLOT(onStarted()));
    connect(ui->centralWidget, SIGNAL(paused()), this, SLOT(onPaused()));
    connect(ui->centralWidget, SIGNAL(endOfPopulation()), this, SLOT(onEnd()));

    ui->mainToolBar->addSeparator();
    mSBWidth = new QSpinBox(this);
    mSBWidth->setMinimum(4);
    mSBWidth->setMaximum(100);
    mSBWidth->setValue(ui->centralWidget->desertWidth());
    connect(mSBWidth, SIGNAL(valueChanged(int)), ui->centralWidget, SLOT(setDesertWidth(int)));
    mSBHeight = new QSpinBox(this);
    mSBHeight->setMinimum(4);
    mSBHeight->setMaximum(100);
    mSBHeight->setValue(ui->centralWidget->desertHeight());
    connect(mSBHeight, SIGNAL(valueChanged(int)), ui->centralWidget, SLOT(setDesertHeight(int)));
    ui->mainToolBar->addWidget(new QLabel(tr("Width:")));
    ui->mainToolBar->addWidget(mSBWidth);
    ui->mainToolBar->addWidget(new QLabel(tr("Height:")));
    ui->mainToolBar->addWidget(mSBHeight);

    ui->mainToolBar->addSeparator();
    mSBUpdateInterval = new QSpinBox(this);
    mSBUpdateInterval->setMinimum(50);
    mSBUpdateInterval->setMaximum(5000);
    mSBUpdateInterval->setValue(ui->centralWidget->updateInterval());
    connect(mSBUpdateInterval, SIGNAL(valueChanged(int)), ui->centralWidget, SLOT(setGenerationUpdateInterval(int)));
    ui->mainToolBar->addWidget(new QLabel(tr("Update interval (msec):")));
    ui->mainToolBar->addWidget(mSBUpdateInterval);

    mCBPresets = new QComboBox(this);
    mCBPresets->addItems(sPresets.keys());
    mCBPresets->setCurrentIndex(-1);
    connect(mCBPresets, SIGNAL(currentIndexChanged(QString)), this, SLOT(onPresetSelected(QString)));
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(new QLabel(tr("Presets:")));
    ui->mainToolBar->addWidget(mCBPresets);

    ui->statusBar->showMessage(tr("You can create or destroy life by clicking cell while generation not in progress."));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onPresetSelected(QString presetName) {
    MainWidget & mw = *ui->centralWidget;
    mw.clear();
    const PPoints & ppoints = sPresets[presetName];

    // find minimum and maximum values of current preset
    QPoint min, max;
    min.rx() = std::numeric_limits<int>::max();
    min.ry() = std::numeric_limits<int>::max();
    max.rx() = std::numeric_limits<int>::min();
    max.ry() = std::numeric_limits<int>::min();
    foreach (const QPoint & pt, ppoints) {
        if (pt.x() > max.x()) max.rx() = pt.x();
        if (pt.x() < min.x()) min.rx() = pt.x();
        if (pt.y() > max.y()) max.ry() = pt.y();
        if (pt.y() < min.y()) min.ry() = pt.y();
    }

    // adjust desert width if need
    // or center this preset horizontally
    int xBase = qAbs(max.x() - min.x());
    if (xBase > ui->centralWidget->desertWidth())
        mSBWidth->setValue(xBase);
    else
        xBase = (ui->centralWidget->desertWidth() - xBase) / 2;

    // adjust desert height
    // or center this preset vertically
    int yBase = qAbs(max.y() - min.y());
    if (yBase > ui->centralWidget->desertHeight())
        mSBHeight->setValue(yBase);
    else
        yBase = (ui->centralWidget->desertHeight() - yBase) / 2;

    // set points
    foreach (const QPoint & pt, ppoints) {
        QPoint p(pt);
        p.rx() += xBase;
        p.ry() += yBase;
        mw.setAsLive(p);
    }

    // redraw widget
    mw.update();
}

void MainWindow::onStarted() {
    enableInputs(false);
    ui->statusBar->showMessage(tr("Started."));
}

void MainWindow::onPaused() {
    enableInputs(true);
    ui->statusBar->showMessage(tr("Paused."));
}

void MainWindow::onEnd() {
    enableInputs(true);
    ui->statusBar->showMessage(tr("End of execution."));
}

void MainWindow::enableInputs(bool flag) {
    ui->actionStart->setVisible(flag);
    ui->actionNext->setVisible(flag);
    ui->actionPause->setVisible(!flag);
    mSBWidth->setEnabled(flag);
    mSBHeight->setEnabled(flag);
    mCBPresets->setEnabled(flag);
}
