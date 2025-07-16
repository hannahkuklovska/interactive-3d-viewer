#pragma once
#include <QtWidgets>
#include "Shape.h"



class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	QPainter* painter = nullptr;
    uchar* data = nullptr; //jednotlivé bajty obrázku, budeme pracovať s jednotlivými bajtami obrázku

    QColor globalColor = QColor(200,149,237);  // RGB color (light pinkish)
    int algType = 0;
    int InterpolationType = 1;


	bool drawLineActivated = false;

    QVector<QPoint> linePoints = {QPoint(0, 0), QPoint(0, 0)};



    QPoint drawPolygonBegin = QPoint(0, 0);


    //bool drawCircleActivated = false;

    //QVector<QPoint> circlePoints;
    bool isFilled = false;

    QVector<Shape> layers;
    std::vector<std::vector<int>> zBuffer;
    int zCounter = 0; // rastie s každým novým objektom







public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

    void setGlobalColor(QColor color) { globalColor = color; }
    QColor getGlobalColor() { return globalColor; }
    void setInterpolationType(int Interpolation) { InterpolationType = Interpolation; }

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

    //rôzne hodnoty intenzít
	void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
    bool isInside(int x, int y) { return (x >= 0 && y >= 0 && x < img->width() && y < img->height()) ? true : false; }
    bool isInside(QPoint point);

    bool isObjectInside(QVector<QPoint> object);
    double distance(QPoint a, QPoint b);




    //Line ------------
    void drawLine(QPoint start, QPoint end, QColor color, int algType, int z);
    void clearLinePoints() {linePoints.clear();}
    QVector<QPoint>& getLinePoints() {return linePoints;}

    void DDA_X(QPoint start, QPoint end, double m, QColor color, int z);
    void DDA_Y(QPoint start, QPoint end, double m, QColor color, int z);
    void DDA(QPoint start, QPoint end, QColor color, int z);

    void BRESENHAM_X(QPoint start, QPoint end, QColor color, double m, int z);
    void BRESENHAM_Y(QPoint start, QPoint end, QColor color, double m, int z);
    void BRESENHAM(QPoint start, QPoint end, QColor color, int z);

    void setDrawLineBegin(QPoint begin) { linePoints[0] = begin; }
    QPoint getDrawLineBegin() { return linePoints[0]; }
    void setDrawLineEnd(QPoint end) { linePoints[1] = end; }
    QPoint getDrawLineEnd() { return linePoints[1]; }

	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

    QVector<QPoint> sutherlandHodgmanLeftClipping(QVector<QPoint> polygon, double xmin);
    //Line ------------


    //Polygon-------
    void drawPolygon(const QVector<QPoint>& points, QColor borderColor, QColor fillColor, int algType, int z);
    //Polygon-------


    //Circle-----
    void BRESENHAMcircle(int userX, int userY, int r, QColor borderColor, QColor fillColor, int isFilled, int z);
    //Circle-----




    void cyrusBeckClip(QPoint P1, QPoint P2, QPoint& clippedP1, QPoint& clippedP2);

    //Clipping

    QVector<QPoint> sutherlandHodgmanClipping(QVector<QPoint> polygon);



    //Fill polygon
    void fillPolygon(QVector<QPoint> polygon, QColor fillColor, int z);

    //Fill triangle
    void scanLineTriangle(QVector<QPoint>&T, QColor color, int InterpolationType, int z);
    void scanLineDown(QPoint T0, QPoint T1, QPoint T2, int InterpolationType,  QColor C0, QColor C1, QColor C2, int z);
    void scanLineUp(QPoint T0, QPoint T1, QPoint T2, int InterpolationType, QColor C0, QColor C1, QColor C2, int z);



    //Krivky Bezier
    void drawBezier(QVector<QPointF>& points,QColor color, int algType, int z);
    void addBezierPoint(QPointF point);




    //interpolation
    //používané
    QColor nearestNeighbor2(QPoint P, QPoint T0, QPoint T1, QPoint T2, QColor C0,QColor C1, QColor C2);
    QColor Barycentric2(QPoint P, QPoint T0, QPoint T1, QPoint T2, QColor C0,QColor C1, QColor C2);



	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img->bits(); }
	void setPainter() { painter = new QPainter(img); }
    void setisFilled(bool filled) { isFilled = filled; }

	int getImgWidth() { return img->width(); };
	int getImgHeight() { return img->height(); };


    QVector<QPoint> sutherlandHodgmanHrana(QVector<QPoint> polygon, int boundary, bool isX, bool isMin);



    //void moveObject(QVector<QPoint>& Points,QPoint delta, QColor color, int algType);
    void moveObject(Shape& shape, QPoint delta);
    void rotateObject(QVector<QPoint>& Points, double angle, QColor color, int algType, bool isClockwise);
    //void scaleObject(QVector<QPoint>& Points, double scaleX, double scaleY, QColor color, int algType);

	void clear();
    void delete_objects();

    void drawShape(const Shape& shape);
    void drawRectangle(QVector<QPoint> corners, QColor borderColor, QColor fillColor, bool filled, int algType, int z);
    void debugPrintLayers();
    void addShapeToLayers(const Shape& shape)
    {
        layers.append(shape);
        qDebug() << "Shape added to layers: type=" << shape.type
                 << ", points=" << shape.points
                 << ", zIndex=" << shape.zIndex;

    }

    bool saveToJson(const QString& fileName);
    bool loadFromJson(const QString& fileName);
    void drawAllLayers();
    QVector<Shape>& getLayers() { return layers; } //referencia, priamo objekt
    void fillCircle(int userX, int userY, int r, QColor color, int z);
    void setPixelZ(int x, int y, const QColor& color, int z);
    int getZCounterAndIncrement() { return zCounter++; } //dá z, ale aj zväčší
    int getZCounter() { return zCounter; } //celkové z
    void setZCounter(int newZcounter) { zCounter = newZcounter; } //celkové z
    void deleteLayers() { layers.clear(); }
    void moveLayerUp(int index);
    void moveLayerDown(int index);













public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};
