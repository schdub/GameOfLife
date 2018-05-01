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
    if (pCell != NULL) {
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
        qreal dw = mDesertSize.width() * mCellSize.width();
        qreal dh = mDesertSize.height() * mCellSize.height();
        QPointF pb(0, 0), pe(0, dh);
        while (pb.x() < dw) {
            pb.rx() += mCellSize.width();
            pe.rx() += mCellSize.width();
            p.drawLine(pb, pe);
        }
        pb.rx() = 0;
        pb.ry() = 0;
        pe.rx() = dw;
        pe.ry() = 0;
        while (pb.y() < dh) {
            pb.ry() += mCellSize.height();
            pe.ry() += mCellSize.height();
            p.drawLine(pb, pe);
        }
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
    if (oldSize == 0) {
        mCells.resize(newSize);
        // clear values for new ones
        if (newSize > oldSize)
            memset(mCells.data() + oldSize, 0, (newSize - oldSize) * sizeof(bool));
    } else {
        // were added new cells
        if (mDesertSize.height() != mDesertSizeOld.height()) {
            mCells.resize(newSize);
        } else {
            Q_ASSERT(mDesertSize.width() != mDesertSizeOld.width());
            int diff = mDesertSize.width() - mDesertSizeOld.width();
            bool addingNewOne = diff > 0;
            if (addingNewOne) {
                // add new cell(s)
                mCells.resize(newSize);
                bool * pSrc = mCells.data() + oldSize - mDesertSizeOld.width();
                bool * pDst = mCells.data() + newSize - mDesertSize.width();
                bool * pEnd = mCells.data();
                for (;;) {
                    for (int i = 0; i < diff; ++i)
                        pDst[mDesertSizeOld.width() + i] = false;
                    if (pSrc == pEnd) break;
                    memmove(pDst, pSrc, mDesertSizeOld.width() * sizeof(bool));
                    pSrc -= mDesertSizeOld.width();
                    pDst -= mDesertSize.width();
                }
            } else {
                // remove cell(s)
                bool * pSrc = mCells.data() + mDesertSizeOld.width();
                bool * pDst = mCells.data() + mDesertSize.width();
                bool * pEnd = mCells.data() + mCells.size();
                for (;;) {
                    if (pSrc == pEnd) break;
                    memmove(pDst, pSrc, mDesertSize.width() * sizeof(bool));
                    pSrc += mDesertSizeOld.width();
                    pDst += mDesertSize.width();
                }
                mCells.resize(newSize);
            }
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

static int loopPos(int pos, int mod) {
    return (pos >= 0) ? pos % mod : mod - (-pos) % mod;
}

bool * MainWidget::cellAt(int x, int y) {
    x = loopPos(x, mDesertSize.width());
    y = loopPos(y, mDesertSize.height());
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
    mDesertSizeOld = mDesertSize;
    mDesertSize.rwidth() = value;
    updateCellSize(size());
    desertAlloc();
    update();
}

void MainWidget::setDesertHeight(int value) {
    mDesertSizeOld = mDesertSize;
    mDesertSize.rheight() = value;
    updateCellSize(size());
    desertAlloc();
    update();
}

void MainWidget::updateCellSize(const QSize & size) {
    qreal t;
    t  = size.width();
    t /= mDesertSize.width();
    mCellSize.setWidth(t);
    t  = size.height();
    t /= mDesertSize.height();
    mCellSize.setHeight(t);
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
