#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwidget.h"

#include <QLabel>
#include <QSpinBox>
#include <QStatusBar>
#include <QComboBox>
#include <QMap>

typedef QList<QPoint> PPoints;
struct Preset {
    QSize defaultSize;
    PPoints points;
    Preset() : defaultSize(21, 21) {}
    Preset(const PPoints & pp) : defaultSize(21, 21), points(pp) {}
};

static QMap<QString, Preset> sPresets;

void MainWindow::initPresets() {
    sPresets.clear();
    sPresets["Clear"]    = Preset();
    sPresets["Glider"]   = Preset( { {10,9}, {11,10}, {11,11}, {10,11}, {9,11} });
    sPresets["Exploder"] = Preset( { {8,8}, {8,9}, {8,10}, {8,11}, {8,12}, {10,8}, {10,12}, {12,8}, {12,9}, {12,10}, {12,11}, {12,12} });
    sPresets["Tumbler"]  = Preset( { {7,10}, {7,11}, {7,12}, {8,7}, {8,8}, {8,12}, {9,7}, {9,8}, {9,9}, {9,10}, {9,11}, {11,7}, {11,8}, {11,9}, {11,10}, {11,11}, {12,7}, {12,8}, {12,12}, {13,10}, {13,11}, {13,12} } );
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
    const Preset & preset = sPresets[presetName];
    mSBWidth->setValue(preset.defaultSize.width());
    mSBHeight->setValue(preset.defaultSize.height());
    foreach (const QPoint & pt, preset.points)
        mw.setAsLive(pt);
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
