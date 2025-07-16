#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
    bool getIsFilled() const {return isFilled;}

private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;
    //QVector<QPoint> polygonPoints; // Ukladanie bodov polygónu
    //QVector<QPoint> linePoints; // Ukladanie bodov čiary

    bool isMoving = false;       // Indikátor, či sa objekt posúva
    QPoint lastMousePos;

    //Filling
    bool isFilled = false;
    bool isFirstClick = true;
    QPoint center;
    int selectedLayerIndex = -1;
    bool isRectangleFirstClick = true;
    QPoint rectangleStart;
    int lockedLayerIndex = -1;

    //dočasné polia
    QVector<QPoint> polygonTempPoints;
    QVector<QPoint> bezierTempPoints;






	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

    QVector<QPoint>& whichPoints();

    void refreshLayerList();



private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	//Tools slots
	void on_pushButtonSetColor_clicked();

    void on_pushButtonScale_clicked();

    void on_comboBoxInterpolation_currentIndexChanged(int index);
    void on_pushButtonRotate_clicked();
    void on_checkBoxFilled_checkStateChanged(const Qt::CheckState &arg1);
    void on_pushButtonSaveJson_clicked();
    void on_pushButtonLoadJson_clicked();
    void on_pushButtonRedrawLayers_clicked();
    void on_listWidgetLayers_currentRowChanged(int row);

    void on_pushButtonDeleteLayer_clicked();
    void on_pushButtonLayerUp_clicked();
    void on_pushButtonLayerDown_clicked();
    void on_pushButtonSetBorderColor_clicked();
    void on_pushButtonSetFillColor_clicked();
    void on_pushButtonChangeFillinfo_clicked();
};
