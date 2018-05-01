#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"

#include <QLabel>
#include <QStatusBar>
#include <QComboBox>
#include <QMap>
#include <limits>
#include <QSlider>

static int defaultSizeMin = 4;
static int defaultSizeMax = 200;
static int defaultPresetW = 50;
static int defaultPresetH = 50;
static int intervalMin = 50;
static int intervalMax = 1000;

typedef QList<QPoint> PPoints;
static QMap<QString, PPoints> sPresets;

void MainWindow::initPresets() {
    sPresets.clear();
    sPresets["Empty"]    = PPoints();
    sPresets["Glider"]   = PPoints() << QPoint(1,0) << QPoint(2,1) << QPoint(2,2) << QPoint(1,2) << QPoint(0,2);
    sPresets["Exploder"] = PPoints() << QPoint(0,0) << QPoint(0,1) << QPoint(0,2) << QPoint(0,3) << QPoint(0,4) << QPoint(2,0) << QPoint(2,4) << QPoint(4,0) << QPoint(4,1) << QPoint(4,2) << QPoint(4,3) << QPoint(4,4);
    sPresets["Tumbler"]  = PPoints() << QPoint(0,3) << QPoint(0,4) << QPoint(0,5) << QPoint(1,0) << QPoint(1,1) << QPoint(1,5) << QPoint(2,0) << QPoint(2,1) << QPoint(2,2) << QPoint(2,3) << QPoint(2,4) << QPoint(4,0) << QPoint(4,1) << QPoint(4,2) << QPoint(4,3) << QPoint(4,4) << QPoint(5,0) << QPoint(5,1) << QPoint(5,5) << QPoint(6,3) << QPoint(6,4) << QPoint(6,5);
    sPresets["Gosper Glider Gun"] = PPoints() << QPoint(0,2) << QPoint(0,3) << QPoint(1,2) << QPoint(1,3) << QPoint(8,3) << QPoint(8,4) << QPoint(9,2) << QPoint(9,4) << QPoint(10,2) << QPoint(10,3) << QPoint(16,4) << QPoint(16,5) << QPoint(16,6) << QPoint(17,4) << QPoint(18,5) << QPoint(22,1) << QPoint(22,2) << QPoint(23,0) << QPoint(23,2) << QPoint(24,0) << QPoint(24,1) << QPoint(24,12) << QPoint(24,13) << QPoint(25,12) << QPoint(25,14) << QPoint(26,12) << QPoint(34,0) << QPoint(34,1) << QPoint(35,0) << QPoint(35,1) << QPoint(35,7) << QPoint(35,8) << QPoint(35,9) << QPoint(36,7) << QPoint(37,8);
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
    mSBWidth = new QSlider(Qt::Horizontal, this);
    mSBWidth->setMinimum(defaultSizeMin);
    mSBWidth->setMaximum(defaultSizeMax);
    mSBWidth->setValue(ui->centralWidget->desertWidth());
    connect(mSBWidth, SIGNAL(valueChanged(int)), ui->centralWidget, SLOT(setDesertWidth(int)));
    mSBHeight = new QSlider(Qt::Horizontal, this);
    mSBHeight->setMinimum(defaultSizeMin);
    mSBHeight->setMaximum(defaultSizeMax);
    mSBHeight->setValue(ui->centralWidget->desertHeight());
    connect(mSBHeight, SIGNAL(valueChanged(int)), ui->centralWidget, SLOT(setDesertHeight(int)));
    ui->mainToolBar->addWidget(new QLabel(tr("Width:")));
    ui->mainToolBar->addWidget(mSBWidth);
    ui->mainToolBar->addWidget(new QLabel(tr("Height:")));
    ui->mainToolBar->addWidget(mSBHeight);

    ui->mainToolBar->addSeparator();
    mSBUpdateInterval = new QSlider(Qt::Horizontal, this);
    mSBUpdateInterval->setMinimum(intervalMin);
    mSBUpdateInterval->setMaximum(intervalMax);
    mSBUpdateInterval->setValue(ui->centralWidget->updateInterval());
    connect(mSBUpdateInterval, SIGNAL(valueChanged(int)), ui->centralWidget, SLOT(setGenerationUpdateInterval(int)));
    ui->mainToolBar->addWidget(new QLabel(tr("Update interval:")));
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

    // set default desert size for preset
    mSBWidth->setValue(defaultPresetW);
    mSBHeight->setValue(defaultPresetH);

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
    int xBase = qAbs(max.x() - min.x())+1;
    if (xBase < ui->centralWidget->desertWidth()) {
        xBase = (ui->centralWidget->desertWidth() - xBase) / 2;
    } else {
        mSBWidth->setValue(xBase);
        xBase = 0;
    }

    // adjust desert height
    // or center this preset vertically
    int yBase = qAbs(max.y() - min.y())+1;
    if (yBase < ui->centralWidget->desertHeight()) {
        yBase = (ui->centralWidget->desertHeight() - yBase) / 2;
    } else {
        mSBHeight->setValue(yBase);
        yBase = 0;
    }

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
