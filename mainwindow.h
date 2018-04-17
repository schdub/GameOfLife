#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QSpinBox;
class QComboBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onStarted();
    void onPaused();
    void onEnd();
    void onPresetSelected(QString);

private:
    Ui::MainWindow *ui;
    QSpinBox * mSBWidth;
    QSpinBox * mSBHeight;
    QSpinBox * mSBUpdateInterval;
    QComboBox * mCBPresets;

    void enableInputs(bool flag);
    static void initPresets();
};

#endif // MAINWINDOW_H
