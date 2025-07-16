#include   "ViewerWidget.h"
#include <cmath>
#include "Shape.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>





ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);

        //buffer inicializácia
        zBuffer = std::vector<std::vector<int>>(img->height(), std::vector<int>(img->width(), INT_MIN));

		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}

}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete painter;
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	update();

	return true;
}
//Pre bod
bool ViewerWidget::isInside(QPoint point) {
    // Získa šírku a výšku obrázka
    int imgWidth = getImgWidth();
    int imgHeight = getImgHeight();

    // Skontroluj, či bod leží v rámci hraníc obrázka
    if (point.x() >= 0 && point.x() < imgWidth && point.y() >= 0 && point.y() < imgHeight) {
        return true;
    } else {
        return false;
    }
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete painter;
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
{
	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = b;
	data[startbyte + 1] = g;
	data[startbyte + 2] = r;
	data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(255 * valB);
	data[startbyte + 1] = static_cast<uchar>(255 * valG);
	data[startbyte + 2] = static_cast<uchar>(255 * valR);
	data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		size_t startbyte = y * img->bytesPerLine() + x * 4;

		data[startbyte] = color.blue();
		data[startbyte + 1] = color.green();
		data[startbyte + 2] = color.red();
		data[startbyte + 3] = color.alpha();
	}
}


//objekt
bool ViewerWidget::isObjectInside(QVector<QPoint> object)
{
    for (int i = 0; i < object.size(); i++)
    {
        //všetky body musia byť vnútri
        if (!isInside(object[i])) {
            //qDebug() << "Point (" << object[i].x() << "," << object[i].y() << ") is outside.";
            return false;  // Ak nájdeme bod mimo oblasti, okamžite sa vráti "false"
        }
    }
    //qDebug() << "All points are inside.";
    return true;  // Všetky body sú vo vnútri
}


//vzdialenosť
double ViewerWidget::distance(QPoint a, QPoint b)
{
    return sqrt(pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2));
}



void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType, int z)
{


    QPoint clippedstart = start;
    QPoint clippedend = end;

    cyrusBeckClip(start, end, clippedstart,clippedend); //funkcia zisti, ci je ciara vnutri

    start = clippedstart;
    end = clippedend;

    if (start == end)
    {
        return; //len bod
    }

    if (algType == 0) //Algoritmus DDA
    {
        DDA(start, end, color, z);

    }


    else if (algType == 1) // Algoritmus Bresenham
    {
        BRESENHAM(start, end, color, z);
    }

    update();
}



/*
void ViewerWidget::drawPolygon(QColor borderColor, QColor fillColor, int algType)
{

    QColor C0(255, 0, 0);  // Červená
    QColor C1(0, 255, 0);  // Zelená
    QColor C2(0, 0, 255);  // Modrá

    if (polygonPoints.size() < 2)
    {
        qDebug() << "Not enough points to draw.";
        return;
    }

    qDebug() << polygonPoints.size();

    if (polygonPoints.size() == 3) //trojuholnik
    {
        clear(); //vyčistí plátno
        QPoint T0 = polygonPoints[0];
        QPoint T1 = polygonPoints[1];
        QPoint T2 = polygonPoints[2];
        if (isFilled == true) //UI
        {
            scanLineTriangle(polygonPoints, InterpolationType);
        }
        //scanLineTriangle(polygonPoints, InterpolationType);
        qDebug() << "3 Points for polygon";
    }

    // Ak je polygón väčší ako trojuholník, vyplníme ho
    if (polygonPoints.size() > 3 && isFilled == true)
    {
        fillPolygon(polygonPoints, globalColor);
    }


    qDebug() << "Drawing polygon with" << polygonPoints.size() << "points.";

    // Skontrolujem, či je polygón vo vnútri oblasti
    QVector<QPoint> clippedPolygon = polygonPoints;

    if (!isObjectInside(polygonPoints)) {
        // Ak nie je, použije sa orezanie
        clear();
        clippedPolygon = sutherlandHodgmanClipping(polygonPoints);
        if (clippedPolygon.isEmpty()) {
            qDebug() << "Clipped polygon is empty. Exiting.";
            return;  // Orezanie zlyhalo alebo polygón je prázdny
        }
    }

    // Ak sa objekt po orezaní nezmenil, kreslím pôvodný objekt
    if (clippedPolygon != polygonPoints) {
        qDebug() << "Drawing clipped polygon." << "size" << clippedPolygon.size();
    } else {
        qDebug() << "Drawing original polygon.";
    }

    /*
    for (int i = 0; i < clippedPolygon.size(); i++) {
        qDebug() << "Point clipped " << i << ": (" << clippedPolygon[i] << ")";

    }
*/

 /*   if (!drawCircleActivated && clippedPolygon.size() == 3)
    {
        clear();
        qDebug() << "clipped, now is a triangle.";
        if (isFilled == true)
        {
            scanLineTriangle(clippedPolygon, InterpolationType);
        }
        //scanLineTriangle(clippedPolygon, InterpolationType);

    }
*/
/*
    if (!drawCircleActivated && clippedPolygon.size() > 3)
    {
        if (isFilled == true)
        {
            fillPolygon(clippedPolygon, globalColor);
        }
        //fillPolygon(clippedPolygon, globalColor);
    }

    // Kreslenie orezaného alebo pôvodného polygónu
    for (int i = 0; i < clippedPolygon.size() - 1; i++) {
        drawLine(clippedPolygon[i], clippedPolygon[i + 1], color, algType);
    }

    //posledná čiara
    if (clippedPolygon.size() > 2) {
        drawLine(clippedPolygon.last(), clippedPolygon.first(), color, algType);
    }


}
*/
void ViewerWidget::drawPolygon(const QVector<QPoint> &points, QColor borderColor, QColor fillColor, int algType, int z)
{
    if (points.size() < 2)
    {
        qDebug() << "Not enough points to draw.PolygonDraw";
        return;
    }

    QVector<QPoint> clippedPolygon = points;

    if (!isObjectInside(points))
    {
        clippedPolygon = sutherlandHodgmanClipping(points);

        if (clippedPolygon.isEmpty())
        {
            qDebug() << "Clipped polygon is empty. Exiting. DrawPolygon";
            return;
        }
    }

    // Výplň
    if (isFilled)
    {
        //trojholník
        if (clippedPolygon.size() == 3)
        {
            scanLineTriangle(clippedPolygon, fillColor, InterpolationType, z);
        }
        else if (clippedPolygon.size() > 3)
        {
            fillPolygon(clippedPolygon, fillColor, z);
        }
    }

    // Hrany
    for (int i = 0; i < clippedPolygon.size() - 1; i++)
    {
        drawLine(clippedPolygon[i], clippedPolygon[i + 1], borderColor, algType, z);
    }

    //prvé s posledným
    if (clippedPolygon.size() > 2)
    {
        drawLine(clippedPolygon.last(), clippedPolygon.first(), borderColor, algType, z);
    }


}






//Move
/*
void ViewerWidget::moveObject(QVector<QPoint>& Points,QPoint delta, QColor color, int algType)
{
    if (Points.isEmpty()) {
        qDebug() << "No points to move!";
        return;
    }

    // Posunutie polygónu
    clear();
    qDebug() << "Number of points before move: " << Points.size();
    //pripočíta každému bodu posunutie delta, podľa posunutia myši
    for (int i = 0; i < Points.size(); i++)
    {
        Points[i] += delta;
    }

    //qDebug() << "Number of points after move: " << Points.size(); // Debug


    // Ak máme len dva body, vykreslíme čiaru
    if (Points.size() == 2) {
        qDebug() << "Moving two points, drawing line";
        drawLine(Points[0], Points[1], color, algType);
    } else {
        qDebug() << "draw again, move, draw polygon";
        drawPolygon(color,color, algType);
    }

    update();
}
*/

//priamo objekt upravujem
void ViewerWidget::moveObject(Shape& shape, QPoint delta)
{
    if (shape.points.isEmpty())
    {
        qDebug() << "No points in shape to move!";
        return;
    }

    // Posun všetkých bodov, pripočítam deltu
    for (int i = 0; i < shape.points.size(); ++i)
    {
        shape.points[i] += delta;
    }


    //podľa enum hodnoty
    if (shape.type == Line)
    {
        if (shape.points.size() >= 2)
        {
            drawLine(shape.points[0], shape.points[1], shape.borderColor, shape.algorithmType, shape.zIndex);
        }
    }

    else if (shape.type == Rectangle && shape.points.size() == 4)
    {
        drawRectangle(
            shape.points,
            shape.borderColor,
            shape.fillColor,
            shape.filled,
            shape.algorithmType,
            shape.zIndex
            );
    }

    else if (shape.type == Polygon)
    {

        globalColor = shape.borderColor;
        isFilled = shape.filled;
        drawPolygon(shape.points,shape.borderColor, shape.fillColor, shape.algorithmType, shape.zIndex);
    }
    else if (shape.type == Circle)
    {
        if (shape.points.size() >= 2)
        {
            QPoint center = shape.points[0];
            QPoint edge = shape.points[1];
            int r = (int)(distance(center, edge) + 0.5);
            BRESENHAMcircle(center.x(), center.y(), r,
                            shape.borderColor, shape.fillColor, shape.filled, shape.zIndex);
        }
    }
    else if (shape.type == Bezier)
    {

        globalColor = shape.borderColor;

        //konverzia na QPointF z QPoint, nové pole, dočasné
        QVector<QPointF> floatPoints;
        for (int i = 0; i < shape.points.size(); ++i)
            floatPoints.append(QPointF(shape.points[i]));

        drawBezier(floatPoints, shape.borderColor, shape.algorithmType, shape.zIndex);

    }

    update(); // prekreslenie widgetu
}





//Line ALGORITMY------------
void ViewerWidget::DDA(QPoint start, QPoint end, QColor color, int z)
{
    int dx = end.x() - start.x();
    int dy = end.y() - start.y();
    double m;

    //ošetrenie vertikálnych čiar
    //dy nemusím, nenastane delenie nulou
    if (dx==0)
    {
        m = __DBL_MAX__;
    }
    else
    {
        m = (double)dy/dx; //sledujem, ktore zmeny sú väčšie
    }

    // Ak je sklony menší než 1, riadiaca os je x, sklon menší ako 45 st
    if (-1 < m && m < 1)
    {
        //ked je otocena normalne, v smere osi, zvačšujúce sa x
        if (start.x() < end.x())
        {
            DDA_X(start, end, m, color, z);
        }

        else
        {
            DDA_X(end, start, m, color, z);
        }
    }

    // Ak je sklony väčší než 1, riadiaca os je Y, väčší ako 45 stupňov
    else
    {
        //otočená správne, y sa zvyšuje
        if (start.y() < end.y())
        {
            DDA_Y(start, end, m, color, z);
        }
        else
        {
            DDA_Y(end, start,m, color, z);
        }
    }
}


// Algoritmus DDA pre riadiacu os X, sklon úsečky < 45
// |m| < 1
void ViewerWidget::DDA_X(QPoint start, QPoint end, double m, QColor color, int z)
{
    //prvý bod
    setPixelZ(start.x(), start.y(), color, z);

    int x;
    double y = start.y();

    //x posúvam o 1, y o smernicu
    for (x = start.x(); x < end.x(); x++)
    {

        y += m; // Zvyšujeme y podľa smernice m
        //y je potrebné zaokrúhliť, je double
        setPixelZ(x, static_cast<int>(y+0.5), color, z);
    }


}


// Algoritmus DDA pre riadiacu os Y
// |m| > 1, sklon úsečky je > 45
void ViewerWidget::DDA_Y(QPoint start, QPoint end,  double m,  QColor color, int z)
{
    setPixelZ(start.x(), start.y(), color, z);
    int y;
    double x = start.x();

    //y zvyšujem o 1, x o 1/m
    for (y = start.y(); y < end.y(); y++)
    {

        x += (1 / m);
        setPixelZ(static_cast<int>(x+0.5), y, color, z); //y int
    }



}

void ViewerWidget::BRESENHAM(QPoint start, QPoint end, QColor color, int z)
{
    int dx = end.x() - start.x();
    int dy = end.y() - start.y();

    double m;

    if (dx==0)
    {
        m = __DBL_MAX__;
    }
    else
    {
        m = (double)dy/dx; //sledujem, ktore zmeny sú väčšie
    }


    if (-1 < m && m < 1)
    {
        if (start.x() < end.x())
        {
            BRESENHAM_X(start, end, color,m, z);
        }
        else
        {
            BRESENHAM_X(end, start, color,m, z);
        }
    }

    else
    {
        if(start.y() < end.y())
        {
            BRESENHAM_Y(start, end, color, m, z);
        }

        else
        {
            BRESENHAM_Y(end, start, color,m, z);
        }
    }


}

//os x je dominantná, väčšie zmeny tam
void ViewerWidget::BRESENHAM_X(QPoint start, QPoint end, QColor color, double m, int z) {



    //if (m > 0 && m < 1) // úsečka stúpa doprava
    if (m > 0)
    {

        int dx = end.x() - start.x();
        int dy = end.y() - start.y();
        int k1 = 2*dy; // len x++
        int p = 2*dy - dx; //rozhodovací bod, celočíselný, počiatočný stav
        int x = start.x();
        int y = start.y();
        int k2 = 2*dy - 2*dx; // diagonálny krok (x++, y++)

        setPixelZ(x,y,color, z);

        for (; x < end.x(); x++) //iterujem po x, až kým neprídem po koniec
        {
            if(p>0) //v smere y++, bližšie bod (yi + 1)
            {
                y++; //ideme diagonálne, hore
                p += k2;
            }

            else //bližšie bod (yi)
            {
                p+=k1; //nemením y, len posun po x, ideme vodorovne
            }
            setPixelZ(x,y,color, z);
        }
    }
    //úsečka klesá doprava
    //pohybuje sa doprava, ale klesá, y sa znižuje
    //k1 a k2 sú upravené pre klesanie


    //if (m > -1 && m < 0) // úsečka klesá doprava
    else
    {
        int dx = end.x() - start.x();
        int dy = end.y() - start.y();
        int k1 = 2 * dy;
        int k2 = 2 * dy + 2 * dx;
        int p = 2 * dy + dx;

        int x = start.x();
        int y = start.y();

        setPixelZ(x,y,color, z);

        for (; x < end.x(); x++)
        {
            if(p<0) //príliíš vysoko, znížim y
            {
                y--; //diagonálny krok
                p += k2;
            }

            else
            {
                p+=k1; //nemením y, iba x++
            }
            setPixelZ(x,y,color,z);
        }

    }


}

//pohyb primárne po y
void ViewerWidget::BRESENHAM_Y(QPoint start, QPoint end, QColor color, double m, int z) {
    if(m > 1) //stúpa hore, ideme po y, x sa nemení tak výrazne
    //if (m > 0)
    {
        int dx = end.x() - start.x();
        int dy = end.y() - start.y();
        int k1 = 2 * dx;
        int k2 = 2 * dx - 2 * dy;
        int p = 2 * dx - dy;

        int x = start.x();
        int y = start.y();

        setPixelZ(x, y, color, z);

        for (; y < end.y(); y++) //iterujem po y
        {
            if(p>0) //(bližšie xi + 1)
            {
                x++; //krok diagonálne
                p += k2;
            }

            else
            {
                p+=k1; //len y++, x nič
            }
            setPixelZ(x,y,color, z);
        }
    }

    if (m < -1) //klesá, strmo nadol, x klesá nadol
    //else
    {
        int dx = end.x() - start.x();
        int dy = end.y() - start.y(); // dy < 0
        int k1 = 2 * dx;
        int k2 = 2 * dx + 2 * dy;
        int p = 2 * dx + dy;

        int x = start.x();
        int y = start.y();

        setPixelZ(x, y, color, z);

        for (; y < end.y(); y++)
        {
            if(p<0) //klesá x
            {
                x--; //krok diagonálne
                p += k2;
            }

            else
            {
                p+=k1; //len y++
            }
            setPixelZ(x,y,color, z);
        }

    }


}

//Line ALGORITMY------------


void ViewerWidget::BRESENHAMcircle(int userX, int userY, int r, QColor borderColor, QColor fillColor, int isFilled, int z)
{

    int x = 0;
    int y = r;
    int p = 1 - r; // Inicializačný rozhodovací parameter, či ďalší bod bude "bližšie" k kružnici zvnútra alebo zvonku
    int dvaX = 3;
    int dvaY = 2 * r - 2;

    if(isFilled)
    {
        fillCircle(userX, userY, r, fillColor, z);
    }


    //ak p > 0, midpoint je vonku, beriem vnútorný bod
    //p < 0, midpoint je vnútri, beriem vonkajší

    //kružnica stredovo aj osovo súmerná
    //od osi y smerom ku kružnici
    while (x <= y) {
        //rozdelenie na 8 častí
        // Premiestnenie stredu kružnice z (0,0) do (userX, userY)
        setPixelZ(userX + x, userY + y,borderColor, z);
        setPixelZ(userX - x, userY + y, borderColor, z);
        setPixelZ(userX + x, userY - y, borderColor, z);
        setPixelZ(userX - x, userY - y, borderColor, z);
        setPixelZ(userX + y, userY + x, borderColor, z);
        setPixelZ(userX - y, userY + x, borderColor, z);
        setPixelZ(userX + y, userY - x,borderColor, z);
        setPixelZ(userX - y, userY - x, borderColor, z);

        if (p > 0) { //bod bližšie k vnútru kružnice, zníženie y, posunutie smerom k stredu
            p -= dvaY;
            y--; //musim sa posunut v y smere nadol
            dvaY -= 2;
        }

        p += dvaX; //zostávam v tom istom riadku, y nemením
        x++; //posunutie doprava
        dvaX += 2;

    }


    update();
}

void ViewerWidget::fillCircle(int userX, int userY, int r, QColor color, int z)
{
    int x = 0;
    int y = r;
    int p = 1 - r;
    int dvaX = 3;
    int dvaY = 2 * r - 2;

    while (x <= y) {
        // Vykreslim horizontálne čiary medzi zrkadlovými bodmi na rovnakej výške
        for (int i = -x; i <= x; i++) {
            setPixelZ(userX + i, userY + y, color,z);
            setPixelZ(userX + i, userY - y, color, z);
        }
        //pre rovnaké x

        for (int i = -y; i <= y; i++) {
            setPixelZ(userX + i, userY + x, color, z);
            setPixelZ(userX + i, userY - x, color, z);
        }

        if (p > 0) {
            p -= dvaY;
            y--;
            dvaY -= 2;
        }

        p += dvaX;
        x++;
        dvaX += 2;
    }

    update();
}




//Clip line
//ktorá úsečka vnútri okna
void ViewerWidget::cyrusBeckClip(QPoint P1, QPoint P2, QPoint& clippedP1, QPoint& clippedP2)
{
    QVector<QPoint> E ={ //pole vrcholov okna
        QPoint(0, 0),
        QPoint(0, img->height()),
        QPoint(img->width(), img->height()),
        QPoint(img->width(), 0)
    };

    if(isInside(P1) && isInside(P2))
    {

        clippedP1 = P1;
        clippedP2 = P2;
        return; //Oba vnútri

    }


    double tl = 0, tu = 1; //hranice orezávania, tu -> koniec iteracie
    //smerový vektor úsečky
    QPoint d = P2 - P1; //vektor posunu úsečky


    for (int i = 0; i < 4; i++)
    {
        //prechádzam cez všetky hrany/spojnice edges
        QPoint n = E[(i+1)%4] - E[i]; //Nasledujúci - aktuálny bod hrany, aby mi nepreslo 4 tak %4, aj E[0] - E[3]
        n = QPoint(n.y(), -n.x()); //vnútorná normála na hranu
        QPoint w = P1 - E[i]; //vektor z bodu hrany do bodu P1

        double dn = QPoint::dotProduct(n,d);
        double wn = QPoint::dotProduct(n,w);

        if (dn != 0) // Úsečka nie je rovnobežná s hranou
        {
            double t = -wn / dn; //kde úsečka pretína hranu

            if (dn > 0 && t <= 1) //ostrý uhol, vystupuje
            {
                if(tl < t)
                {
                    tl = t; //najdem max
                }
            }

            else if (dn < 0 && 0 <= t) //tupý uhol, vstupuje
            {
                if (tu > t)
                {
                    tu = t; //najdem min
                }
            }

        }

    }

    //t0 -> bod je v P1, t1 -> bod je v P2
    if (tl == 0 && tu == 1) //úsečka sa nemenila, zostáva
    {
        clippedP1 = P1;
        clippedP2 = P2;
        //qDebug() << "== P1: " << P1 << "P2: " << P2 << "clippedP1: " << clippedP1 << "clippedP2: " << clippedP2;
    }



    else if (tl < tu) //výpočet orezaných bodov
    {

        //clippedP1 = P1 + d*tl;
        //clippedP2 = P1 + d*tu;

        //P'= P1 + t.d, vzorec pre orezaný bod
        clippedP1 = QPoint(
            static_cast<int>(P1.x() + d.x() * tl + 0.5),
            static_cast<int>(P1.y() + d.y() * tl + 0.5)
            );
        clippedP2 = QPoint(
            static_cast<int>(P1.x() + d.x() * tu + 0.5),
            static_cast<int>(P1.y() + d.y() * tu + 0.5)
            );
        //qDebug() << "P1: " << P1 << "P2: " << P2 << "clippedP1: " << clippedP1 << "clippedP2: " << clippedP2;


    }

}


// Sutherland-Hodgman pre ľavú hranicu
QVector<QPoint> ViewerWidget::sutherlandHodgmanLeftClipping(QVector<QPoint> polygon, double xmin) {

    QVector<QPoint> clippedPolygon;
    int n = polygon.size();


    if (n == 0) return clippedPolygon;  // Ak nie sú body, nič netreba orezávať

    QPoint S = polygon[n - 1];  // Posledný bod ako začiatočný, spojenie posledného, prvého

    //prvá iterácia porovnávam prvý s posledným

    for (int i = 0; i < n; i++) {
        QPoint V = polygon[i];

        if (V.x() >= xmin) // Bod je vnútri alebo na hranici orezávacej oblasti
        {
            if (S.x() >= xmin)
            { // Predchádzajúci aj aktuálny bod sú vo vnútri
                clippedPolygon.push_back(V); //pridám do zoznamu

            }
            else
            {
                // Predchádzajúci bod je mimo, ale aktuálny je vo vnútri
                QPoint P = QPoint(xmin,
                                  S.y() + (xmin - S.x()) * (polygon[i].y() - S.y()) / (polygon[i].x() - S.x()));
                clippedPolygon.push_back(P); // Pridáme priesečník
                clippedPolygon.push_back(V); // Pridáme aktuálny bod

            }

        }
        else
        {
            if (S.x() >= xmin) //aktuálny vonku, predchádzajúci vnútri
            {
                QPoint P = QPoint(xmin,
                                  S.y() + (xmin - S.x()) * (polygon[i].y() - S.y()) / (polygon[i].x() - S.x()));

                clippedPolygon.push_back(P); //pridám len priesečník
            }


        }

        S = V;  // Posuň sa na ďalší bod
    }
    return clippedPolygon; //nové pole



}


QVector<QPoint> ViewerWidget::sutherlandHodgmanClipping(QVector<QPoint> polygon)
{

    int xmin = 5;
    int ymin = 5;
    int xmax = this->width()-5;
    int ymax = this->height()-5;

    QVector<QPoint> clippedPolygon = polygon;


    // Iterujeme cez každú hranicu
    for (int i = 0; i < 4; i++)
    {
        // Orezávanie podľa aktuálnej hrany (ľavá, dolná, pravá, horná)
        clippedPolygon = sutherlandHodgmanLeftClipping(clippedPolygon, xmin);

        // Otočenie bodov po orezaní (zámena x a y a zmena znamienka), rotácia o 90st
        for (QPoint &p : clippedPolygon)
        {
            int temp = p.x(); //kolmý (u,v) -> (v, -u)
            p.setX(p.y());
            p.setY(-temp);
        }

        // Otočenie hraníc orezávacej oblasti, o 90 st
        int temp = xmin;
        xmin = ymin;
        ymin = -xmax;
        xmax = ymax;
        ymax = -temp;
    }


    return clippedPolygon;


}



void ViewerWidget::scanLineTriangle(QVector<QPoint>& T ,QColor color, int InterpolationType, int z)
{
    QColor C0 = QColor(255, 0, 0);  // Red
    QColor C1 = QColor(0, 255, 0);  // Green
    QColor C2 = QColor(0, 0, 255);  // Blue


    // Usporiadanie bodov podľa y-ovej súradnice vzostupne a sekundárne podľa x-ovej
    //T0 musí mať najmenšiu ynovú suradnicu
    for (int i = 0; i < T.size() - 1; ++i) {
        for (int j = 0; j < T.size() - 1 - i; ++j) {
            // Porovnáme dva body podľa ich y a x súradníc
            if (T[j].y() > T[j + 1].y() || (T[j].y() == T[j + 1].y() && T[j].x() > T[j + 1].x())) {
                // Ak sú v nesprávnom poradí, prehodíme ich
                //výmena pozícií
                std::swap(T[j], T[j + 1]);
            }
        }
    }



    //T0 najmenšie y

    // Ak majú T0 a T1 rovnakú y-ovú súradnicu, riešime len spodný trojuholník
    // T0 a T1 sú hore
    // T0 nalavo, T1 napravo (rastuce x), T2 dole
    if (T[0].y() == T[1].y())
    {
        qDebug() << "Lower Triangle: " << T[2] << T[1] << T[0];
        //scanLineDown(T[0], T[1],T[2], InterpolationType, C0, C1, C2, z);
        scanLineDown(T[0], T[1],T[2], InterpolationType, color, color, color, z);
        qDebug() << "only bottom";
    }

    // Ak majú T1 a T2 rovnakú y-ovú súradnicu, riešime len horný trojuholník
    //T0 je hore (má nejmenšiu y súradnicu
    // T2 a T1 na jednej urovni, T1 nalavo, T2 napravo, T0 hore
    else if (T[1].y() == T[2].y())
    {
        //top triangle
        qDebug() << "Upper Triangle: " << T[2] << T[1] << T[0];
        //scanLineUp(T[0], T[1],T[2], InterpolationType,C0, C1,  C2, z);
        scanLineUp(T[0], T[1],T[2], InterpolationType,color, color, color, z);
        qDebug() << "iba horný";

    }

    else
    {

        //rozelenie na 2
        //sklon spojnice T0-T2
        double m = (double)(T[2].y() - T[0].y()) / (double)(T[2].x() - T[0].x());
        //leži medzi T0 a T2 vo výške T1
        QPoint P(((double)(T[1].y() - T[0].y()) / m) + T[0].x(), T[1].y());


        qDebug() << "Delim na P::" << P;
        qDebug() << "Horny trojuholník: " << T[0] << T[1] << P;
        qDebug() << "Dolný trojuholník: " << T[1] << P << T[2];

        //relatívna pozícia bodu P medzi bodmi T[0] a T[2] podľa y, P "moje nove T1"
        //ako veľmi je P blízko k T0 alebo T2

        // ak P = T0, alpha = 0
        //ak P = T2 alpha je 1
        double alpha = (double)(P.y() - T[0].y()) / (double)(T[2].y() - T[0].y());


        // najbližší vrchol k P, podľa neho farba
        double d0 = distance(P, T[0]);
        double d1 = distance(P, T[1]);
        double d2 = distance(P, T[2]);
        QColor CP;

        //Farba
        if (InterpolationType == 0)
        {

            if (d0 <= d1 && d0 <= d2) CP = C0; // najbližší k  T[0]
            else if (d1 <= d0 && d1 <= d2) CP = C1; // najbližší k  T[1]
            else CP = C2; // najbližší k  T[2]
        }
        else //Baryc, lineárna interpolácia farieb
        {
            //pomer medzi T0 a T2
            CP = QColor(
                (1 - alpha) * C0.red() + alpha * C2.red(),
                (1 - alpha) * C0.green() + alpha * C2.green(),
                (1 - alpha) * C0.blue() + alpha * C2.blue()
                );
        }

        if (T[1].x() < P.x()) //je napravo
        {
            // Triangle 1: T[0], T[1], P
            // Triangle 2: T[1], P, T[2]
            //scanLineUp(T[0], T[1], P, InterpolationType, C0, C1,  CP, z);
            //scanLineDown(T[1], P, T[2], InterpolationType, C1, CP,  C2, z);
            scanLineUp(T[0], T[1], P, InterpolationType, color, color, color, z);
            scanLineDown(T[1], P, T[2], InterpolationType, color, color,color, z);
            //qDebug() << "obe Scanline Triangel 1";
        }

        else //P je naľavo
        {
            // Triangle 1: T[0], P, T[1]
            // Triangle 2: P, T[1], T[2]
            //scanLineUp(T[0], P, T[1], InterpolationType,C0, CP,  C1, z);
            //scanLineDown(P, T[1], T[2], InterpolationType,CP,  C1,  C2, z);
            scanLineUp(T[0], P, T[1], InterpolationType,color, color, color, z);
            scanLineDown(P, T[1], T[2], InterpolationType,color, color, color, z);

            //qDebug() << "obe Scanline Triangel  2";
        }

        return;
    }




}
//hore T0 - T1
// dole k T2
void ViewerWidget::scanLineDown(QPoint T0, QPoint T1, QPoint T2, int InterpolationType,  QColor C0, QColor C1, QColor C2, int z)
{
    //čím nižšie, tým je užší
    //T0->T2 ľavá hrana
    //T1->T2 Phrana

    //nakonci sa zbehnú v 1 bode

    QPoint e1_start = T0;
    QPoint e1_end = T2;

    QPoint e2_start = T1;
    QPoint e2_end = T2;


    //smernice, 1/m, dy/dx
    double we1 =  ((double)(T2.x() - T0.x()) / (double)(T2.y()-T0.y()));
    double we2 =  ((double)(T2.x() - T1.x()) / (double)(T2.y()-T1.y()));


    double x1 = e1_start.x(); // double x1 = T0.x();
    double x2 = e2_start.x(); // double x2 = T1.x();



    //zvyšujem y
    for (int y = e1_start.y(); y < e1_end.y(); y++)
    {
        if (x1 != x2)
        {
            for (int x = int(x1); x <= int(x2); x++)
            {

                //QColor color = (InterpolationType == 0) ?


                                    //nearestNeighbor2(QPoint(x, y), T0, T1, T2,C0, C1, C2) :
                                    //Barycentric2(QPoint(x, y), T0, T1, T2,C0, C1, C2);

                setPixelZ(x, y, C0, z);


            }
        }
        x1 += we1;  // zvýšenie o smernicu
        x2 += we2;
    }

}

QColor ViewerWidget::nearestNeighbor2(QPoint P, QPoint T0, QPoint T1, QPoint T2, QColor C0,QColor C1, QColor C2)
{
    //vzdialenosti
    double d0 = std::pow(T0.x() - P.x(), 2) + std::pow(T0.y() - P.y(), 2);
    double d1 = std::pow(T1.x() - P.x(), 2) + std::pow(T1.y() - P.y(), 2);
    double d2 = std::pow(T2.x() - P.x(), 2) + std::pow(T2.y() - P.y(), 2);



    QColor Cp;

    // Určíme farbu na základe najbližšieho bodu
    if (d0 <= d1 && d0 <= d2) {
        Cp = C0;
    }
    else if (d1 <= d0 && d1 <= d2) {
        Cp = C1;
    }
    else {
        Cp = C2;
    }

    return Cp;
}


QColor ViewerWidget::Barycentric2(QPoint P, QPoint T0, QPoint T1, QPoint T2, QColor C0,QColor C1, QColor C2)
{
    //𝑃 = 𝜆0𝑇 0 + 𝜆1𝑇 1 + 𝜆2𝑇 2, pre ľubovoľný bod v troj.
    //𝜆0 + 𝜆1 + 𝜆2 = 1
    //obsah celeho cez normalu
    // Menovateľ barycentrickej interpolácie

    //obsah celého A
    double denom = ((T1.x() - T0.x())  * (T2.y() - T0.y()) - (T1.y() - T0.y()) * (T2.x() - T0.x()));
    denom = abs(denom);  // Uistíme sa, že je vždy kladný

    // Nominátory pre jednotlivé lambda hodnoty, A0/A, A1/A
    double nomLambda0 = ((T1.x() - P.x()) * (T2.y() - P.y()) - (T1.y() - P.y()) * (T2.x() - P.x()));
    double nomLambda1 = ((T0.x() - P.x()) * (T2.y() - P.y()) - (T0.y() - P.y()) * (T2.x() - P.x()));

    // Absolútna hodnota nominátorov
    nomLambda0 = abs(nomLambda0);
    nomLambda1 = abs(nomLambda1);

    // Výpočet lambda hodnôt
    double lambda0 = nomLambda0 / denom;
    double lambda1 = nomLambda1 / denom;
    double lambda2 = 1.0 - lambda0 - lambda1; // Pretože suma všetkých lambda hodnôt by mala byť 1

    // Sčíta sa vážená hodnota farieb
    int red = lambda0 * C0.red() + lambda1 * C1.red() + lambda2 * C2.red();
    int green = lambda0 * C0.green() + lambda1 * C1.green() + lambda2 * C2.green();
    int blue = lambda0 * C0.blue() + lambda1 * C1.blue() + lambda2 * C2.blue();

    // Vracia výslednú farbu
    return QColor(red, green, blue);
}


//rozbiehajú sa z jedneho bodu
//horný
//T0 hore, T1 - T2 rovnobežné
void ViewerWidget::scanLineUp(QPoint T0, QPoint T1, QPoint T2, int InterpolationType,  QColor C0, QColor C1, QColor C2, int z)
{
    //čím vyššie, tým je užší, z bodu do tvoreneho, hrany sa rozchádzajú

    QPoint e1_start = T0;
    QPoint e1_end = T1;

    QPoint e2_start = T0;
    QPoint e2_end = T2;

    double we1 =  (double)(T1.x() - T0.x()) / (double)(T1.y()-T0.y());
    double we2 =  (double)(T2.x() - T0.x()) / (double)(T2.y()-T0.y());



    double x1 = e1_start.x();
    double x2 = e2_start.x();

    //int yStart = e1_start.y();
    //int yMax = e1_end.y();

    for (int y = e1_start.y(); y < e1_end.y(); y++)
    {
        if (x1 != x2)
        {
            for (int x = int(x1); x <= int(x2); x++)
            {

                //QColor color = (InterpolationType == 0) ?
                                   //nearestNeighbor2(QPoint(x, y), T0, T1, T2, C0, C1, C2) :
                                   //Barycentric2(QPoint(x, y), T0, T1, T2, C0, C1, C2);

                setPixelZ(x, y, C0, z);


            }
        }
        x1 += we1;  // zvýšenie o smernicu
        x2 += we2;
    }
}





void ViewerWidget::fillPolygon(QVector<QPoint> polygon, QColor fillColor, int z)
{
    if (polygon.size() < 3) // Oprava kontroly minimálneho počtu vrcholov
    {
        return;
    }

    // 1. Vytvorenie zoznamu hrán
    struct Edge {
        QPoint start; //začiatočný bod hrany
        QPoint end; //koncový bod hrany
        int dy; //zmena y, y_end - y_start, počet riadkov do ktorých hrana zasahuje
        double x; //aktuálna súradnica x
        double w; //Smernica (o koľko sa x zmení pri posune o 1 v osi y)



    };

    QVector<Edge> edges; //všetky nevodorovné hrany

    //vytváram hrany, nie vodorovné
    for (int i = 0; i < polygon.size(); i++) {
        QPoint p1 = polygon[i];
        QPoint p2 = polygon[(i + 1) % polygon.size()]; //spojenie posledného s prvým

        if (p1.y() == p2.y())
            continue; // Preskočim vodorovné hrany

        if (p1.y() > p2.y()) //počiatočný musí byť menší, chcem zhora nadol
        {
            QPoint temp = p1;
            p1 = p2;
            p2 = temp;
        }


         //o koľko sa zmení x pri posune y o 1
        double w = (double)(p2.x() - p1.x()) / (double)(p2.y() - p1.y()); //smernica

        //koncový bod o 1 nadol, aby sa mi nezdvojili hrany, priesečník neuvažujem
        p2.setY(p2.y() - 1);

        Edge edge = {p1, p2, p2.y() - p1.y(), (double)p1.x(), w};
        edges.push_back(edge); //pridám do hrán
    }

    // Zoradenie hrán podľa y, vzostupne
    for (int i = 0; i < edges.size() - 1; i++)
    {
        //o hranu dalej
        for (int j = i + 1; j < edges.size(); j++)
        {
            if (edges[i].start.y() > edges[j].start.y()) {
                Edge temp = edges[i];
                edges[i] = edges[j];
                edges[j] = temp;
            }
        }
    }

    int y_min = edges[0].start.y();
    int y_max = y_min;

    for (int i = 0; i < edges.size(); i++)
    {
        if (edges[i].end.y() > y_max)
        {
            y_max = edges[i].end.y(); //najväčší bod polygónu
        }
    }


    //pridanie hrán ktoré začínajú v danom y
    //každý index odpovedá konkrétnemu y


    //priradim podla y
    //list hrán TH[i] = {Edge(y,x,m), Edge(y,x,m)}
    QVector<QList<Edge>> TH(y_max - y_min + 1); //TH[i] viac možných hrán


    for (int i =0 ; i < edges.size();i++)
    {
         //každú hranu do správneho zoznamu v TH, podľa toho aké ma y
        int index = edges[i].start.y() - y_min;
        TH[index].push_back(edges[i]);
    }


    //zoznam akívnych hrán
    //obsahuje všetky aktuálne hrany pre dané ymin
    QVector<Edge> ZAH; //veľkosť sa mení dynamicky
    int y = y_min; //prvý riadok


    //iterujem všetky y riadky, od y_min po y_max
    for (int i = 0; i < TH.size(); i++)
        {
        for (int j = 0; j < TH[i].size(); j++)
        {
            ZAH.push_back(TH[i][j]); //pridáva všetky hrany, ktoré začínajú na danom y, i, riadok
        }

        // Zoradenie aktívnych hrán podľa x, zľava doprava, zvyšujúce sa x
        for (int j = 0; j < ZAH.size() - 1; j++)
        {
            for (int k = j + 1; k < ZAH.size();k++)
            {
                if (ZAH[j].x > ZAH[k].x)
                {
                    Edge temp = ZAH[j];
                    ZAH[j] = ZAH[k];
                    ZAH[k] = temp;
                }
            }
        }

        // Kontrola počtu hrán (musí byť párny)
        //algoritmus by nemal dvojice začiatok/koniec
        if (ZAH.size() % 2 != 0)
        {
            qDebug() << "Chyba: Nepárny počet hrán v aktívnom zozname";
            throw std::runtime_error("Scanline Error,ZAH not even");
        }

        // Vyplnenie medzi pármi hrán
        //každý pár určuje úsek
        for (int j = 0; j < ZAH.size(); j += 2)
        {
            int xStart = static_cast<int>(ZAH[j].x+0.5); //celé čísla
            int xEnd = static_cast<int>(ZAH[j + 1].x+0.5);


            for (int x = xStart; x <= xEnd; x++)
            {
                setPixelZ(x, y, fillColor, z); //vyplní medzi dvomi hranami na danom y
            }
        }

        // Aktualizácia aktívnych hrán
        for (int j = 0; j < ZAH.size(); j++)
        {
            if (ZAH[j].dy == 0)
            { //hrana už nie je v zozname, jej výskyt už je 0
                ZAH.removeAt(j);
                j--;
            }
            else
            {
                ZAH[j].x += ZAH[j].w; //posunutie o smernicu, Posun x podľa smernice
                ZAH[j].dy--;
            }
        }
        y++; // Posuniem sa na ďalší riadok
    }
}





//prechádza prvým a posledným bodom
//aproximačná
void ViewerWidget::drawBezier(QVector<QPointF> &points, QColor color, int algType, int z)
{
    //hlbšie do y sa počet xsových zmenšuje, robia sa LK predošlých
    int n = points.size();
    if (n < 2) return;
    // Orezanie: aspoň jeden bod musí byť vnútri
    bool atLeastOneInside = false;

    for (int i = 0; i < points.size(); i++)
    {
        QPointF pFloat = points[i];          // Získam bod typu QPointF
        QPoint pInt = pFloat.toPoint();      // Prevediem na QPoint

        if (isInside(pInt))                  // či je bod vo vnútri obrázka
        {
            atLeastOneInside = true;         // Aspoň jeden vnútri
            break;                           // nemusím ďalej kontrolovať
        }
    }

    double deltaT = 0.01, t = deltaT; //posuny, rasterizacia
    QPointF Q0 = points[0];
    QPointF Q1;
    QVector<QPointF> temp = points; //všetky riadiace body

    //bod závislý od riadiacich a t

    while (t <= 1) { //pre každé t 1 interpolovany bod
        for (int i = 1; i < n; i++) {
            for (int j = 0; j < n - i; j++) {
                //interpolovanie medzi bodmi podľa t
                //na konci algortimu už je 1 bod -> bod Bezierovej krivky
                //dostanem bod na krivke
                temp[j].setX((1 - t) * temp[j].x() + t * temp[j + 1].x());
                temp[j].setY((1 - t) * temp[j].y() + t * temp[j + 1].y());
            }
        }
        Q1 = temp[0]; // aktuálny bod pre t, na krivke
        //zapamätávam iba kvôli kresleniu priamok

        //"OREZÁVANIE"
        if (isInside(Q0.toPoint()) || isInside(Q1.toPoint())) {
            drawLine(Q0.toPoint(), Q1.toPoint(), color, algType, z); // časť aproximácie krivky
        }

        Q0 = Q1; //pre ďalšiu čiaru
        t += deltaT;
    }

    if (isInside(Q0.toPoint()) || isInside(Q1.toPoint())) {
        drawLine(Q0.toPoint(), Q1.toPoint(), color, algType, z);
    }

}



void ViewerWidget::drawShape(const Shape& shape)
{
    if (shape.type == Line && shape.points.size() >= 2)
    {
        drawLine(shape.points[0], shape.points[1], shape.borderColor, shape.algorithmType, shape.zIndex);
    }


    else if (shape.type == Rectangle && shape.points.size() == 4)
    {
        drawRectangle(
            shape.points,
            shape.borderColor,
            shape.fillColor,
            shape.filled,
            shape.algorithmType,
            shape.zIndex
            );
    }




    else if (shape.type == Polygon)
    {

        globalColor = shape.borderColor;
        isFilled = shape.filled;
        drawPolygon(shape.points,shape.borderColor, shape.fillColor, shape.algorithmType, shape.zIndex);
    }

    else if (shape.type == Circle && shape.points.size() >= 2)
    {
        QPoint center = shape.points[0];
        QPoint radiusPoint = shape.points[1];
        int r = (int)(distance(center, radiusPoint) + 0.5);

        BRESENHAMcircle(center.x(), center.y(), r, shape.borderColor, shape.fillColor, shape.filled, shape.zIndex);
    }


     else if (shape.type == Bezier)
    {

        globalColor = shape.borderColor;
        QVector<QPointF> floatPoints;
        for (int i = 0; i < shape.points.size(); ++i)
            floatPoints.append(QPointF(shape.points[i]));

        drawBezier(floatPoints, shape.borderColor, shape.algorithmType, shape.zIndex);


    }



    }


void ViewerWidget::drawRectangle(QVector<QPoint> corners, QColor borderColor, QColor fillColor, bool filled, int algType, int z)
    {
        if (corners.size() != 4) {
            qDebug() << "Neplatný počet bodov pre obdĺžnik. Očakávané 4.";
            return;
        }

        // Orezanie
        if (!isObjectInside(corners)) {
            QVector<QPoint> clipped = sutherlandHodgmanClipping(corners);
            if (clipped.isEmpty()) {
                qDebug() << "Obdĺžnik mimo okna, nekreslím.";
                return;
            }

            if (filled) {
                fillPolygon(clipped, fillColor, z);
            }

            for (int i = 0; i < clipped.size(); i++)
            {
                //aby som spojila prvý s posledným, ak size = 4, [3], [0]
                drawLine(clipped[i], clipped[(i + 1) % clipped.size()], borderColor, algType, z);
            }
            return;
        }

        // Bez orezania
        if (filled) {
            fillPolygon(corners, fillColor, z);
        }

        for (int i = 0; i < 4; i++)
        {
            drawLine(corners[i], corners[(i + 1) % 4], borderColor, algType, z);
        }
    }



void ViewerWidget::debugPrintLayers()
{
    qDebug() << "==========";
    qDebug() << "DEBUG: Zoznam vrstiev";

    for (int i = 0; i < layers.size(); i++)
    {
        Shape s = layers[i];  // zoberieme jeden tvar

        // Typ ako číslo (napr. 0 = čiara, 1 = obdĺžnik atď.)
        int typ = s.type;

        // Vypíšeme základné údaje
        qDebug() << "Vrstva číslo:" << i;
        qDebug() << "Typ (číslo):" << typ;
        qDebug() << "Body:" << s.points;
        qDebug() << "Z-index (hĺbka):" << s.zIndex;
        qDebug() << "Vyplnené:" << s.filled;
        qDebug() << "--------------------";
    }

    qDebug() << "==========";
}


bool ViewerWidget::saveToJson(const QString& fileName)
{
    QJsonObject rootObj; //{}

    // Uloženie veľkosti plátna, kľúče, width a height
    rootObj["width"] = img->width();
    rootObj["height"] = img->height();


    // Vytvorím JSON pole pre všetky tvary
    QJsonArray shapePole;

    //Json Array naplnený json objektami = shapes/layers, má vlastnosti

    // Prejdem všetky uložené tvary
    for (int i = 0; i < layers.size(); i++)
    {
        Shape shape = layers[i];  // Zoberiem jeden tvar

        QJsonObject shapeObj;

        // Uložím základné vlastnosti tvaru/objektu v poli-----, key: value
        shapeObj["type"] = shape.type;
        shapeObj["zIndex"] = shape.zIndex;
        shapeObj["filled"] = shape.filled;
        shapeObj["algorithm"] = shape.algorithmType;
        shapeObj["borderColor"] = shape.borderColor.name();
        shapeObj["fillColor"] = shape.fillColor.name();

        // Vytvorím JSON pole pre body tvaru, vnorené pole
        QJsonArray bodyPole;

        for (int j = 0; j < shape.points.size(); j++)
        {
            QPoint p = shape.points[j];

            QJsonObject bod;
            bod["x"] = p.x();
            bod["y"] = p.y();

            bodyPole.append(bod); // Pridám bod do poľa
        }

        // Pripojím body k objektu tvaru
        shapeObj["points"] = bodyPole;

        // Pridám celý objekt tvaru do hlavného poľa
        shapePole.append(shapeObj);
    }

    //pole vrstiev
    // pripojIM celé pole vrstiev do hlavného JSON objektu, ROOT
    rootObj["layers"] = shapePole;

    // Vytvorím dokument a zapíšeme ho do súboru
    QJsonDocument doc(rootObj);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    //zapíše JSON dokument ako text do súboru

    file.write(doc.toJson());
    file.close();



    return true; // Úspešné uloženie
}

bool ViewerWidget::loadFromJson(const QString& fileName)
{
    QFile file(fileName);

    // Pokúsim sa otvoriť súbor na čítanie
     if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll(); // Načítam všetko zo súboru
    file.close();


    // Vytvorim JSON dokument z textu, aby bolo možné pracovať s JSN objektmi
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return false;


    // z documentu objekt {}
    QJsonObject rootObj = doc.object(); // Celý JSON ako slovník


    // Nastavenie veľkosti plátna
    //vytiahnem kľuč-hodnota, zo "slovníka
    int w = rootObj["width"].toInt(); //jsvalues
    int h = rootObj["height"].toInt();
    if (w > 0 && h > 0) {
        changeSize(w, h);
    }



    // Vyčistenie existujúcich vrstiev
    layers.clear();

    // Získanie poľa vrstiev (útvarov), z root
    //vytiahnem celé pole - JSN pole
    QJsonArray layerArray = rootObj["layers"].toArray(); //[]


    // Pre každý útvar v poli, 1 vrstva
    for (int i = 0; i < layerArray.size(); i++)
    {
        QJsonObject obj = layerArray[i].toObject(); //{} // jedna vrstva

        // Vytvorim nový tvar
        Shape shape;


        //číselnú hodnotu na enum, int na shapetype
        //(pretypuje) jeden dátový typ na iný
        //počas prekladania, nie behu programu

        // Načítam základné vlastnosti útvaru
        shape.type = static_cast<ShapeType>(obj["type"].toInt()); // premení 1 → Rectangle
        shape.zIndex = obj["zIndex"].toInt();
        shape.filled = obj["filled"].toBool();
        shape.algorithmType = obj["algorithm"].toInt();
        shape.borderColor = QColor(obj["borderColor"].toString());
        shape.fillColor = QColor(obj["fillColor"].toString());

        // Načítaj všetky body
        //pre každý bod načítam x,y -> vytvorím QPoint a pridám do shape.points

        //ako slovník/mapa, x je kľúč
        QJsonArray points = obj["points"].toArray();
        for (int j = 0; j < points.size(); j++)
        {
            //point je object {}
            QJsonObject p = points[j].toObject();
            int x = p["x"].toInt();
            int y = p["y"].toInt();
            shape.points.append(QPoint(x, y));
        }

        // Pridam tento tvar do zoznamu vrstiev (typu shape)
        layers.append(shape);
    }

    // Nastavenie zCounter tak, aby ďalší tvar dostal správny zIndex

    int maxZ = 0;
    for (int i = 0; i < layers.size(); i++)
    {
        if (layers[i].zIndex > maxZ)
        {
            maxZ = layers[i].zIndex;
        }
    }
    zCounter = maxZ + 1;
    qDebug() << "Načítaná veľkosť plátna: " << w << "x" << h;

    return true; // Načítanie úspešné
}

void ViewerWidget::drawAllLayers()
{
    clear(); // Vymažem plátno

    // Pre každý útvar zavolám jeho vykreslenie, podľa vrstvy
    for (int i = 0; i < layers.size(); ++i)
    {
        drawShape(layers[i]);  // priamo pracujeme s originálom
    }

    update();  // Aktualizujem widget

}

void ViewerWidget::setPixelZ(int x, int y, const QColor& color, int z)
{
    //overenie, či sú súradnice mimo
    if (x < 0 || y < 0 || x >= img->width() || y >= img->height()) return;

    //porovnanie s aktuálnou hĺbkou v Z-bufferi na tom mieste

    // Ak je nový pixel „bližšie“, vykreslím ho
    if (z >= zBuffer[y][x])
    {
        setPixel(x,y,color);
        zBuffer[y][x] = z;
    }

     // Inak sa pixel neprekreslí (prekrýva ho bližší objekt)
}

void ViewerWidget::moveLayerDown(int index)
{
    if (index < 0 || index >= layers.size())
        return;

    // zIndex aktuálne vybranej vrstvy
    int currentZ = layers[index].zIndex;

    // Chcem nájsť vrstvu, ktorá je tesne „pod“ touto — čiže má zIndex o 1 mensší
    int targetZ = currentZ - 1;

    for (int i = 0; i < layers.size(); ++i)
    {
        if (layers[i].zIndex == targetZ)
        {
            // Prehodím zIndexy medzi vrstvami
            int temp = layers[index].zIndex;
            layers[index].zIndex = layers[i].zIndex;
            layers[i].zIndex = temp;
            return;
        }
    }
    // Ak žiadna vrstva nemá targetZ, znížim zIndex aktuálnej vrstvy
    layers[index].zIndex = targetZ;
}



void ViewerWidget::moveLayerUp(int index)
{
    if (index < 0 || index >= layers.size())
        return;

    int currentZ = layers[index].zIndex;

    // Chcem nájsť vrstvu, ktorá je tesne „nad“ touto — čiže má zIndex o 1 väčší
    int targetZ = currentZ + 1;

    for (int i = 0; i < layers.size(); ++i)
    {
        if (layers[i].zIndex == targetZ)
        {
            // Prehodím zIndexy: vybraná vrstva <-> vrstva nad ňou
            int temp = layers[index].zIndex;
            layers[index].zIndex = layers[i].zIndex;
            layers[i].zIndex = temp;
            return;
        }
    }

    layers[index].zIndex = targetZ;
}









void ViewerWidget::delete_objects()
{
    linePoints = {QPoint(0, 0), QPoint(0, 0)};
    //linePoints.clear();
    //circlePoints.clear();


    update();
}


void ViewerWidget::clear()
{
	img->fill(Qt::white);
    //zbuffer znova
    zBuffer = std::vector<std::vector<int>>(img->height(), std::vector<int>(img->width(), INT_MIN));
	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}
