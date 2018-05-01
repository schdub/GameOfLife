#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QStack>

class MainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);

    int updateInterval() const;
    int desertWidth() const;
    int desertHeight() const;

protected:
    void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void resizeEvent(QResizeEvent * event);

signals:
    void started();
    void paused();
    void endOfPopulation();

public slots:
    void start();
    void pause();
    void generationUpdate();
    void clear();
    void setAsLive(const QPoint & pt);
    void setGenerationUpdateInterval(int value);
    void setDrawGrid(bool value);
    void setDesertWidth(int value);
    void setDesertHeight(int value);

private:
    bool mDrawGrid;
    QTimer * mTimer;
    QSize mDesertSize;
    QSize mDesertSizeOld;
    QVector<bool> mCells;
    QStack< QPair< int, int > > mDeads;
    QStack< QPair< int, int > > mAlives;
    QSizeF mCellSize;

    QPoint desertPointToScreenPoint(QPoint pt);
    QPoint screenPointToDesertPoint(QPoint pt);
    bool * cellAt(int x, int y);
    bool * cellAt(const QPoint & pt);
    int neighboursCount(int x, int y);
    void updateCellSize(const QSize & size);
    void desertClear();
    void desertAlloc();
};

#endif // MAINWIDGET_H
