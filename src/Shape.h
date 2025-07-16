#ifndef SHAPE_H
#define SHAPE_H

#include <QVector>
#include <QPoint>
#include <QColor>


//Vlastný dátový typ - ShapeType, zoznam platných hodnôt
// hodnoty mapované na čísla

enum ShapeType //nahradím číselné hodnoty menami
{
    Line = 0,
    Rectangle = 1,
    Polygon = 2,
    Circle = 3,
    Bezier = 4
};


struct Shape {
    ShapeType type; // typ tvaru
    QVector<QPoint> points;   // Body, ktoré definujú tvar
    QColor borderColor;      // farba okraja
    QColor fillColor;
    int zIndex;               // Hĺbka pre Z-buffer
    bool filled;              // Či je výplň aktívna
    int algorithmType;        // 0 = DDA, 1 = Bresenham atď.




};

#endif // SHAPE_H

