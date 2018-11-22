#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QLabel>

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
    void on_btn_choosePicture_clicked();

    void on_btn_detect_clicked();

    void replyDetectFinished(QNetworkReply* reply);

    void replyCompareFinished(QNetworkReply* reply);

    void on_btn_pictureBefore_clicked();

    void on_btn_pictureAfter_clicked();

    void on_btn_startCompare_clicked();

private:
    Ui::MainWindow *ui;
    QImage* img;
    QImage* compareImg1;
    QImage* compareImg2;
    QNetworkAccessManager *manager;
    QNetworkAccessManager *compareManager;

    void showImage(QString & imageName, QLabel * & position, QImage * & img);
    void checkImage();
    void postDetectData(QString & qstrImg);
    void postCompareData(QString & qstrImg1, QString & qstrImg2);
    void jsonDetectDataParser(QByteArray & relpyJson);
    void jsonCompareDataParser(QByteArray & relpyJson);
};

#endif // MAINWINDOW_H
