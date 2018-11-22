#include <QVariant>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>
#include <QIODevice>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    img(nullptr),
    compareImg1(nullptr),
    compareImg2(nullptr)
{
    ui->setupUi(this);
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyDetectFinished(QNetworkReply*)));

    compareManager = new QNetworkAccessManager(this);
    connect(compareManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyCompareFinished(QNetworkReply*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*void MainWindow::checkImage()
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart max_face_num; max_face_num.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"max_face_num\""));
    max_face_num.setBody("1"); QHttpPart face_fields; face_fields.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"face_fields\""));
    face_fields.setBody("age,gender,qualities"); QHttpPart image_data; image_data.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\""));
    image_data.setBody("图像的base64编码数据");
    multiPart->append(max_face_num);
    multiPart->append(face_fields); multiPart->append(image_data);
    QUrl url("https://aip.baidubce.com/rest/2.0/face/v1/detect?access_token=24.cbc92b55c4e10c057065857526cd849a.2592000.1525230249.282335-11029510");
    QNetworkRequest request(url);
    networkManager->post(request, multiPart);
}*/

void MainWindow::on_btn_choosePicture_clicked()
{
    QString imageName;
    imageName = QFileDialog::getOpenFileName(this,
                                             "选择图片",
                                             "",
                                             tr("Images (*.png *.bmp *.jpg *.tif *.GIF )"));
    qDebug() << "imageName:" << imageName;

    if(imageName.isEmpty()) {
        qWarning() << "image is empty.";
         return;
    }

    showImage(imageName, ui->label, img);
}

void MainWindow::showImage(QString & imageName, QLabel * & position, QImage * & img)
{
    img=new QImage(imageName);
    QImage* fitImg = new QImage(img->scaled(
                                    position->width(),
                                    position->height(),
                                    Qt::KeepAspectRatio
                                    ));

    position->setPixmap(QPixmap::fromImage(*fitImg));
}

void MainWindow::postDetectData(QString &qstrImg)
{
    qDebug() << "postData IN";
    //设置请求地址
    QString requestUrl = "https://aip.baidubce.com/rest/2.0/face/v3/detect";
    QString accessToken = "24.4d40828a679662874a71f9fc0f818d1a.2592000.1544172414.282335-14622857";
    QUrl url(requestUrl + "?access_token=" + accessToken);
    QNetworkRequest request(url);

    //设置数据提交格式，这个不能自己随便写，每个平台的格式可能不一样，百度AI要求的格式为application/json
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    QJsonObject obj;
    QJsonDocument doc;

    obj.insert("image", qstrImg);
    obj.insert("image_type", "BASE64");
    obj.insert("face_field", "age,beauty,gender,expression,quality");
    obj.insert("max_face_num", 10);
    doc.setObject(obj);

    QByteArray postParam = doc.toJson(QJsonDocument::Compact);
    // qDebug() << "postParam:" << postParam;
    manager->post(request, postParam);
    qDebug() << "postData OUT";
}

void MainWindow::postCompareData(QString &qstrImg1, QString &qstrImg2)
{
    qDebug() << "postCompareData IN";
    //设置请求地址
    QString requestUrl = "https://aip.baidubce.com/rest/2.0/face/v3/match";
    QString accessToken = "24.4d40828a679662874a71f9fc0f818d1a.2592000.1544172414.282335-14622857";
    QUrl url(requestUrl + "?access_token=" + accessToken);
    QNetworkRequest request(url);

    //设置数据提交格式，这个不能自己随便写，每个平台的格式可能不一样，百度AI要求的格式为application/json
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    QJsonObject obj1, obj2;
    QJsonArray arr;
    // QJsonDocument doc;

    obj1.insert("image", qstrImg1);
    obj1.insert("image_type", "BASE64");
    obj2.insert("image", qstrImg2);
    obj2.insert("image_type", "BASE64");

    arr.append(obj1);
    arr.append(obj2);
    // doc.setArray(arr);

    QByteArray postParam = QJsonDocument(arr).toJson();

    /*QStringList list;
    list.append("[");
    list.append(QString("{\"image\":\"%1\",\"image_type\":\"BASE64\",\"liveness_control\":\"NONE\"}").arg(qstrImg1));
    list.append(",");
    list.append(QString("{\"image\":\"%1\",\"image_type\":\"BASE64\",\"liveness_control\":\"NONE\"}").arg(qstrImg2));
    list.append("]");
    QString data = list.join("");*/
    compareManager->post(request, postParam);
    qDebug() << "postCompareData OUT";
}

void MainWindow::jsonDetectDataParser(QByteArray &relpyJson)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(relpyJson, &err);
    QString toShowInfo;

    if(err.error == QJsonParseError::NoError) {
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            //解析反馈的人脸属性结果
            if(obj.contains("result")) {
                QJsonObject resultObj = obj.take("result").toObject();
                //解析人脸个数
                if(resultObj.contains("face_num")) {
                    int faceNum = resultObj.take("face_num").toInt();
                    qDebug()<<"查询到了图片中的人脸个数为："<< faceNum;
                }
                //解析人脸属性
                if(resultObj.contains("face_list")) {
                    QJsonArray faceArray = resultObj.take("face_list").toArray();
                    for(int i = 0; i < faceArray.size(); i++) {
                        QJsonObject faceObj = faceArray.at(i).toObject();

                        // 解析性别
                        if(faceObj.contains("gender")) {
                            QJsonObject genderObj = faceObj.take("gender").toObject();
                            if(genderObj.contains("type")) {
                                QString type = genderObj.take("type").toString();
                                if(type == "male") {
                                    toShowInfo += "性别：男\r\n";
                                } else {
                                    toShowInfo += "性别：女\r\n";
                                }
                            }
                        }

                        // 解析年龄
                        if(faceObj.contains("age")) {
                            int age = faceObj.take("age").toDouble();
                            toShowInfo += "年龄：" + QString("%1").arg(age) + "\r\n";
                        }

                        // 解析颜值
                        if(faceObj.contains("beauty")) {
                            int beauty = faceObj.take("beauty").toDouble();
                            toShowInfo += "颜值：" + QString("%1").arg(beauty) + "\r\n";
                        }

                        // 解析表情
                        if(faceObj.contains("expression")) {
                            QJsonObject expressionObj = faceObj.take("expression").toObject();
                            if(expressionObj.contains("type")) {
                                QString type = expressionObj.take("type").toString();
                                if(type == "smile") {
                                    toShowInfo += "表情：微笑\r\n";
                                } else if(type == "laugh") {
                                    toShowInfo += "表情：大笑\r\n";
                                } else {
                                    toShowInfo += "表情：不笑\r\n";
                                }
                            }
                        }
                        toShowInfo += "\r\n================\r\n";
                    }
                }
            }
        }
    }

    qDebug() << "toShowInfo:" << toShowInfo;
    ui->textBrowser->setText(toShowInfo);
}

void MainWindow::jsonCompareDataParser(QByteArray &relpyJson)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(relpyJson, &err);
    QString toShowInfo;
    if(err.error == QJsonParseError::NoError) {
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            if(obj.contains("result")) {
                QJsonObject resultObj = obj.take("result").toObject();
                if(resultObj.contains("score")) {
                    double score = resultObj.take("score").toDouble();
                    qDebug() << "相似度为：" << score;
                    ui->lb_compareResult->setText(QString("相似度为：%1").arg(score));
                }
            }
        }
    }
}

void MainWindow::on_btn_detect_clicked()
{
    qDebug() << "on_btn_detect_clicked IN";
    // 图片进行base64编码
    QByteArray ba;
    QBuffer buffer(&ba);
    ba.clear();
    buffer.open(QIODevice::WriteOnly);

    img->save(&buffer, "jpg");
    QString qstrImg = QString(ba.toBase64());
    qDebug() << "base64 len:" << qstrImg.length();
    buffer.close();
    postDetectData(qstrImg);
    qDebug() << "on_btn_detect_clicked OUT";
}

void MainWindow::replyDetectFinished(QNetworkReply *reply)
{
    qDebug() << "replyDetectFinished IN";
    QByteArray replyData = reply->readAll();
    reply->close();
    qDebug()<<"reply data is:"<<QString(replyData);

    jsonDetectDataParser(replyData);

    qDebug() << "replyDetectFinished OUT";
}

void MainWindow::replyCompareFinished(QNetworkReply *reply)
{
    qDebug() << "replyCompareFinished IN";
    QByteArray replyData = reply->readAll();
    reply->close();
    qDebug()<<"reply data is:"<<QString(replyData);

    jsonCompareDataParser(replyData);

    qDebug() << "replyCompareFinished OUT";
}

void MainWindow::on_btn_pictureBefore_clicked()
{
    QString imageName;
    imageName = QFileDialog::getOpenFileName(this,
                                             "选择图片",
                                             "",
                                             tr("Images (*.png *.bmp *.jpg *.tif *.GIF )"));
    qDebug() << "imageName:" << imageName;

    if(imageName.isEmpty()) {
        qWarning() << "image is empty.";
         return;
    }

    showImage(imageName, ui->lb_pic1, compareImg1);
}

void MainWindow::on_btn_pictureAfter_clicked()
{
    QString imageName;
    imageName = QFileDialog::getOpenFileName(this,
                                             "选择图片",
                                             "",
                                             tr("Images (*.png *.bmp *.jpg *.tif *.GIF )"));
    qDebug() << "imageName:" << imageName;

    if(imageName.isEmpty()) {
        qWarning() << "image is empty.";
         return;
    }

    showImage(imageName, ui->lb_pic2, compareImg2);
}

void MainWindow::on_btn_startCompare_clicked()
{
    qDebug() << "on_btn_startCompare_clicked IN";
    // 图片进行base64编码
    QByteArray ba1, ba2;
    QString qstrImg1, qstrImg2;
    QBuffer buffer1(&ba1), buffer2(&ba2);
    buffer1.open(QIODevice::WriteOnly);
    buffer2.open(QIODevice::WriteOnly);

    if (compareImg1 != nullptr) {
         compareImg1->save(&buffer1, "jpg");
         qstrImg1 = QString(ba1.toBase64());;
    }

    if (compareImg2 != nullptr) {
        compareImg2->save(&buffer2, "jpg");
        qstrImg2 = QString(ba2.toBase64());
    }

    buffer1.close();
    buffer2.close();


    postCompareData(qstrImg1, qstrImg2);


    qDebug() << "on_btn_startCompare_clicked OUT";
}
