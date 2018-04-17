#include "mainwidget.h"

#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStack>
#include <QPair>

//#include <QDebug>
//#define TRACE qDebug() << __FUNCTION__

static int sDefaultDesertWidth  = 20;
static int sDefaultDesertHeight = 20;
static int sDefaultUpdateInterval = 100;

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mDrawGrid(true)
    , mTimer(new QTimer(this))
    , mDesertSize(sDefaultDesertWidth, sDefaultDesertHeight)
{
    mTimer->setInterval(sDefaultUpdateInterval);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(generationUpdate()));
    desertAlloc();
}

void MainWidget::resizeEvent(QResizeEvent * event) {
    updateCellSize(event->size());
    update();
}

void MainWidget::mousePressEvent(QMouseEvent * event) {
    if (mTimer->isActive()) {
        // ignore press event because life in progress
        return;
    }

    QPoint pt = screenPointToDesertPoint(event->pos());
    //TRACE << event->pos() << pt;
    bool * pCell = cellAt(pt);
    if (pCell != nullptr) {
        *pCell = !(*pCell);
        update();
    }
}

void MainWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    // bg
    p.setBrush(Qt::darkGray);
    p.drawRect(rect());
    // draw grid
    if (mDrawGrid) {
        p.setPen(Qt::gray);
        int dw = mDesertSize.width() * mCellSize.width();
        int dh = mDesertSize.height() * mCellSize.height();
        for (int x = 0; x < dw; x += mCellSize.width())
            p.drawLine(x, 0, x, dh);
        for (int y = 0; y < dh; y += mCellSize.height())
            p.drawLine(0, y, dw, y);
    }
    // draw cells
    p.setPen(Qt::yellow);
    p.setBrush(Qt::yellow);
    bool * pCell = mCells.data();
    for (int y = 0; y < mDesertSize.height(); ++y) {
        for (int x = 0; x < mDesertSize.width(); ++x, ++pCell) {
            if (*pCell == false) continue;
            p.drawRect(x * mCellSize.width()+1,
                       y * mCellSize.height()+1,
                       mCellSize.width()-2,
                       mCellSize.height()-2);
        }
    }
}

// one population generation routine (called by timer on by Next button)

void MainWidget::generationUpdate() {
    bool * pCell = mCells.data();
    mDeads.clear();
    mAlives.clear();
    for (int y = 0; y < mDesertSize.height(); ++y) {
        for (int x = 0; x < mDesertSize.width(); ++x, ++pCell) {
            int count = neighboursCount(x, y);
            if (*pCell == true) {
                // alive
                if (count < 2 || count > 3) {
                    mDeads.push(QPair<int,int>(x, y));
                }
            } else {
                // dead
                if (count == 3) {
                    mAlives.push(QPair<int,int>(x, y));
                }
            }
        }
    }
    // any moves? then stop execution
    if (mDeads.isEmpty() && mAlives.isEmpty()) {
        if (mTimer->isActive()) {
            mTimer->stop();
            emit endOfPopulation();
        }
        return;
    }
    // mark dead ones
    while (!mDeads.isEmpty()) {
        const QPair<int, int> & pt = mDeads.top();
        bool * pCell = cellAt(pt.first, pt.second);
        if (pCell) {
            *pCell = false;
        }
        mDeads.pop();
    }
    // .. and those who were born
    while (!mAlives.isEmpty()) {
        const QPair<int, int> & pt = mAlives.top();
        bool * pCell = cellAt(pt.first, pt.second);
        if (pCell) {
            *pCell = true;
        }
        mAlives.pop();
    }
    // repaint widget
    update();
}

// start generating process
void MainWidget::start() {
    Q_ASSERT(!mTimer->isActive());
    mTimer->start();
    emit started();
}

// pause generating process
void MainWidget::pause() {
    Q_ASSERT(mTimer->isActive());
    mTimer->stop();
    emit paused();
}

// clear all cells in the Desert
void MainWidget::desertClear() {
    memset(mCells.data(), 0, mCells.size() * sizeof(bool));
}

// allocate memory for cells in Desert
void MainWidget::desertAlloc() {
    int newSize = mDesertSize.width() * mDesertSize.height();
    int oldSize = mCells.size();
    if (newSize != oldSize) {
        mCells.resize(newSize);
        if (newSize > oldSize) {
            // clear values for new ones
            memset(mCells.data() + oldSize, 0, (newSize - oldSize) * sizeof(bool));
        }
    }
}

void MainWidget::clear() {
    Q_ASSERT(!mTimer->isActive());
    desertClear();
    update();
}

// set cell as alive (need by Presets)
void MainWidget::setAsLive(const QPoint & pt) {
    bool * pCell = cellAt(pt);
    Q_ASSERT(pCell);
    *pCell = true;
}

void MainWidget::setGenerationUpdateInterval(int value) {
    mTimer->setInterval(value);
}

QPoint MainWidget::desertPointToScreenPoint(QPoint pt) {
    pt.rx() *= mCellSize.width();
    pt.ry() *= mCellSize.height();
    return pt;
}

QPoint MainWidget::screenPointToDesertPoint(QPoint pt) {
    pt.rx() /= mCellSize.width();
    pt.ry() /= mCellSize.height();
    return pt;
}

bool * MainWidget::cellAt(const QPoint & pt) {
    return cellAt(pt.x(), pt.y());
}

bool * MainWidget::cellAt(int x, int y) {
    if (x >= mDesertSize.width() || x < 0 ||
        y >= mDesertSize.height() || y < 0) {
        return nullptr;
    }
    return mCells.data() + y * (mDesertSize.width()) + x;
}

int MainWidget::neighboursCount(int x, int y) {
    int count = 0;
    bool* n[8];
    n[0] = cellAt(x-1, y+1);
    n[1] = cellAt(x-1, y);
    n[2] = cellAt(x-1, y-1);
    n[3] = cellAt(x,   y-1);
    n[4] = cellAt(x+1, y-1);
    n[5] = cellAt(x+1, y);
    n[6] = cellAt(x+1, y+1);
    n[7] = cellAt(x,   y+1);
    for (int i = 0; i < 8; ++i) {
        if (n[i] && *(n[i]) == true) ++count;
    }
    return count;
}

void MainWidget::setDrawGrid(bool value) {
    mDrawGrid = value;
    update();
}

void MainWidget::setDesertWidth(int value) {
    mDesertSize.rwidth() = value;
    updateCellSize(size());
    desertAlloc();
    update();
}

void MainWidget::setDesertHeight(int value) {
    mDesertSize.rheight() = value;
    updateCellSize(size());
    desertAlloc();
    update();
}

void MainWidget::updateCellSize(const QSize & size) {
    mCellSize.setWidth(size.width() / mDesertSize.width());
    mCellSize.setHeight(size.height() / mDesertSize.height());
    //TRACE << mCellSize;
}

int MainWidget::updateInterval() const {
    return mTimer->interval();
}

int MainWidget::desertWidth() const {
    return mDesertSize.width();
}

int MainWidget::desertHeight() const {
    return mDesertSize.height();
}
