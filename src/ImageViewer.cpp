#include "ImageViewer.h"
#include <QListWidgetItem>


ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
    vW = new ViewerWidget(QSize(500, 700));

	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);
    globalColor = QColor(200,149,237);
	QString style_sheet = QString("background-color: #%1;").arg(globalColor.rgba(), 0, 16);
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
    ui->checkBoxFilled->setChecked(false);



}


// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
    //line------
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked())
	{
        if (w->getDrawLineActivated()) //druhý bod
        {
            w->setDrawLineEnd(e->pos());
            QPoint p1 = w->getDrawLineBegin();
            QPoint p2 = e->pos();

            Shape line;
            line.type = Line;
            line.points = { p1, p2 };
            line.borderColor = globalColor;
            line.fillColor = Qt::transparent;
            line.filled = false;
            line.zIndex = w->getZCounterAndIncrement();
            line.algorithmType = ui->comboBoxLineAlg->currentIndex();

            w->drawShape(line);
            w->addShapeToLayers(line);
            refreshLayerList(); //imgViewer UI
            w->setDrawLineActivated(false);

		}
        else { //prvý bod
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
            w->setPixel(e->pos().x(), e->pos().y(), w->getGlobalColor());
			w->update();

		}
	} 
    //line------

    //Rectangle----
    // Kreslenie obdĺžnika
    if (e->button() == Qt::LeftButton && ui->toolButtonRectangle->isChecked())
    {
        if (isRectangleFirstClick)
        {
            rectangleStart = e->pos();
            w->setPixel(rectangleStart.x(), rectangleStart.y(), globalColor);
            w->update();
            isRectangleFirstClick = false;
        }
        else
        {
            QPoint secondPoint = e->pos();

            int x1 = rectangleStart.x();
            int y1 = rectangleStart.y();
            int x2 = secondPoint.x();
            int y2 = secondPoint.y();

            int left, right, top, bottom;

            if (x1 < x2) {
                left = x1;
                right = x2;
            } else {
                left = x2;
                right = x1;
            }

            if (y1 < y2) {
                top = y1;
                bottom = y2;
            } else {
                top = y2;
                bottom = y1;
            }

            Shape rect;
            rect.type = Rectangle;

            // Vytvor 4 rohy v poradí
            rect.points.append(QPoint(left, top));     // horný ľavý
            rect.points.append(QPoint(right, top));    // horný pravý
            rect.points.append(QPoint(right, bottom)); // dolný pravý
            rect.points.append(QPoint(left, bottom));  // dolný ľavý

            rect.borderColor = globalColor;
            rect.fillColor = globalColor;
            rect.filled = isFilled;
            rect.zIndex = w->getZCounterAndIncrement(); //získa z od vW, a pridá ++
            rect.algorithmType = ui->comboBoxLineAlg->currentIndex();

            w->drawShape(rect); //nakreslí podľa typu
            w->addShapeToLayers(rect); //pridá do vektra vrstiev
            refreshLayerList(); //refresne tabuľku

            isRectangleFirstClick = true;
        }
    }
     //Rectangle----



    // Kreslenie kružnice
    if (e->button() == Qt::LeftButton && ui->toolButtonDrawCircle->isChecked())
    {

        if (isFirstClick)
        {
            // Prvý klik: Nastavím stred kružnice
            center = e->pos();
            w->setPixel(center.x(), center.y(), w->getGlobalColor()); // Označíme bodkou
            w->update();
            isFirstClick = false; // Čakám na druhý klik
        }
        else
        {
            // Druhý klik: Vypočíta polomer a vykreslí kružnicu, dist
            int radius = std::sqrt(std::pow(e->pos().x() - center.x(), 2) + std::pow(e->pos().y() - center.y(), 2));


            Shape circle;
            circle.type = Circle;
            circle.points = { center, e->pos() };  // stred a bod na obvode
            circle.borderColor = globalColor;
            circle.fillColor = globalColor;
            circle.filled = isFilled;
            circle.zIndex = w->getZCounterAndIncrement();
            circle.algorithmType = 0;

            w->drawShape(circle);
            w->addShapeToLayers(circle);
            refreshLayerList(); //imgViewer UI
            isFirstClick = true; //znova
        }


    }

    //Posúvanie MOVE
    if (e->button() == Qt::LeftButton && ui->toolButtonMove->isChecked())
    {

        isMoving = true; //posúvam až pokým nie je mouseeventrelease
        lastMousePos = e->pos(); // Uložíme aktuálnu pozíciu myši

        if (lockedLayerIndex < 0) //ktoré posúvam, zistím index
        {
            lockedLayerIndex = selectedLayerIndex;

        }


    }


    //Polygon----
    if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked())
    {
        polygonTempPoints.append(e->pos()); //dočasný vektor

    }
    if (e->button() == Qt::RightButton && ui->toolButtonDrawPolygon->isChecked())
    {

        if (polygonTempPoints.size() > 2)

        {

            //vytvorím útvar
            Shape polygon;
            polygon.type = Polygon;
            polygon.points = polygonTempPoints;
            polygon.borderColor = globalColor;
            polygon.fillColor = globalColor;
            polygon.filled = isFilled;
            polygon.zIndex = w->getZCounterAndIncrement(); //++z
            polygon.algorithmType = ui->comboBoxLineAlg->currentIndex();

            w->drawShape(polygon);
            w->addShapeToLayers(polygon);
            refreshLayerList(); //imgViewer UI
            polygonTempPoints.clear(); //vymažem dočasné body

        }
        else
        {
            qDebug() << "Nedostatočný počet bodov na polygon.";
        }

    }


    //Krivka Bezier----
    if (e->button() == Qt::LeftButton && ui->toolButtonBezier->isChecked())
    {


        bezierTempPoints.append(e->pos()); //dočasný vektor
        //update(); // Aktualizuje vykreslenie okna

    }

    if (e->button() == Qt::RightButton && ui->toolButtonBezier->isChecked())
    {

        Shape bezier;
        bezier.type = Bezier;
        bezier.points = bezierTempPoints;
        bezier.borderColor = globalColor;
        bezier.fillColor = Qt::transparent;
        bezier.filled = false;
        bezier.zIndex = w->getZCounterAndIncrement(); //++z
        bezier.algorithmType = 0;



        w->drawShape(bezier); //nakreslím
        w->addShapeToLayers(bezier); //pridám do vektora
        refreshLayerList(); //imgViewer UI
        bezierTempPoints.clear(); // Vyčistenie dočasného vektora

    }


}

//Znamenie, že sa objekt prestal posúvať
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
    QMouseEvent* e = static_cast<QMouseEvent*>(event);
    if (e->button() == Qt::LeftButton)
    {
        isMoving = false;
    }
}

//keď sa hýbe, daný objekt sa prekresluje
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
    QMouseEvent* e = static_cast<QMouseEvent*>(event);
    if (isMoving && ui->toolButtonMove->isChecked())
    {
        if (lockedLayerIndex >= 0 && lockedLayerIndex < w->getLayers().size())
        {

            QPoint delta = e->pos() - lastMousePos; //posun
            QVector<Shape>& layers = w->getLayers();        // referencia na celý vektor
            Shape& shape = layers[lockedLayerIndex];
            w->moveObject(shape, delta);
            w->drawAllLayers();
            refreshLayerList();
        }


        lastMousePos = e->pos(); // vždy aktualizujem pozíciu na tú predtým
    }


}



void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}

void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
    QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
    //QVector<QPoint>& points = w->whichPoints(); // Získanie bodov objektu (polygón/čiara)
    //if (points.isEmpty()) {
        //qDebug() << "No object to scale.";
        //return;
    //}

    //if (ui->toolButtonBezier->isChecked())
    //{
       // qDebug() << "Žiaden scale pre krivky.";
     //return;
    //}
    /*

    if (!ui->toolButtonBezier->isChecked())
    {
        if (wheelEvent->angleDelta().y() > 0)
        {
           // w->scaleObject(points, 1.25, 1.25,w->getGlobalColor(), ui->comboBoxLineAlg->currentIndex());
        }
        else
        {
            //w->scaleObject(points, 0.75, 0.75,w->getGlobalColor(), ui->comboBoxLineAlg->currentIndex());

        }
    }
*/

}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}




void ImageViewer::on_actionClear_triggered()
{
    //vymaže polia, čiaru nastaví na (0,0)
	vW->clear();
    vW->delete_objects();
    // Vymaže vrstvy
    vW->deleteLayers();
    // Vymaže položky v QListWidget
    ui->listWidgetLayers->clear();
    //ui->listWidgetLayers->setCurrentIndex(-1);
    vW->setZCounter(0);




}

void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

//globálna
void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
    if (newColor.isValid())
    {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
        globalColor = newColor; //nová globálna farba
        vW->setGlobalColor(newColor);
	}
}



void ImageViewer::on_pushButtonRotate_clicked()
{
    if (lockedLayerIndex < 0 || lockedLayerIndex >= vW->getLayers().size()) //nech je valídna
    {
        QMessageBox::warning(this, "Rotácia", "Nie je vybratá žiadna vrstva.");
        return;
    }

    int uholStupne = ui->spinBoxSpin->value();
    double uholRadiany = uholStupne * M_PI / 180.0;

    QVector<Shape>& vrstvy = vW->getLayers(); //referancia, upravujem priamo, získam celé pole
    Shape& shape = vrstvy[lockedLayerIndex]; //index i vrstvy v poli, tú čo chcem

    if (shape.points.size() < 2) return;

    QPoint stred = shape.points[0]; // okolo prvého bodu

    QVector<QPoint> noveBody; //dočasné pole

    for (int i = 0; i < shape.points.size(); i++) {

        QPoint bod = shape.points[i];

        // Posun na súradnice relatívne k stredu
        int dx = bod.x() - stred.x();
        int dy = bod.y() - stred.y();

        //vkonám rotáciu bodov
        double otoceneX = dx * cos(uholRadiany) - dy * sin(uholRadiany);
        double otoceneY = dx * sin(uholRadiany) + dy * cos(uholRadiany);


        // Posun späť k stredu, + zaokrúhlenie
        int vysledneX = (int)(otoceneX + stred.x() + 0.5);
        int vysledneY = (int)(otoceneY + stred.y() + 0.5);

        noveBody.append(QPoint(vysledneX, vysledneY)); // pridám do dočasného pola
    }

    // Uložím nové body, upravený útvar
    shape.points = noveBody;

    vW->drawAllLayers(); //prekreslím

}




void ImageViewer::on_pushButtonScale_clicked()
{
    // Overíme, či je vybraná vrstva
    if (lockedLayerIndex < 0 || lockedLayerIndex >= vW->getLayers().size()) {
        QMessageBox::warning(this, "Škálovanie", "Nie je vybratá žiadna vrstva.");
        return;
    }

    // Načítam hodnoty zo vstupu, koeficienty škálovania
    double scaleX = ui->doubleSpinBoxScaleX->value();
    double scaleY = ui->doubleSpinBoxScaleY->value();

    if (scaleX == 0.0 || scaleY == 0.0)
    {
        QMessageBox::warning(this, "Neplatné hodnoty", "Škálovanie nemôže byť nulové.");
        return;
    }

    // Získame objekt Shape, referencia, priamo objekt
    QVector<Shape>& vrstvy = vW->getLayers();
    Shape& shape = vrstvy[lockedLayerIndex]; //vyberiem chcený

    // Ak nemá body, nič neškálujem
    if (shape.points.isEmpty()) return;

    // "Stred škálovania" bude prvý bod, dočasne do (0,0)
    QPoint center = shape.points[0];

    // Pre každý bod v tvare
    for (int i = 0; i < shape.points.size(); i++)
    {
        QPoint p = shape.points[i];

        // Posunieme bod na súradnice vzhľadom na stred
        double x = p.x() - center.x();
        double y = p.y() - center.y();

        // Vynásobíme škálovacím koeficientom
        double scaledX = x * scaleX;
        double scaledY = y * scaleY;

        // Posunieme späť a zapíšeme späť do poľa
        shape.points[i].setX(static_cast<int>(scaledX + center.x())); //spätný posun
        shape.points[i].setY(static_cast<int>(scaledY + center.y()));
    }

    // Znova prekreslíme všetko
    vW->drawAllLayers();
}



void ImageViewer::on_pushButtonSaveJson_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "Uložiť vrstvy", "", "JSON (*.json)");
    if (!path.isEmpty())
    {
        if (vW->saveToJson(path)) //bool
        {
            QMessageBox::information(this, "Uložené", "Vrstvy boli uložené.");
        } else {
            QMessageBox::warning(this, "Chyba", "Uloženie zlyhalo.");
        }
    }
}

void ImageViewer::on_pushButtonLoadJson_clicked()
{

    QString path = QFileDialog::getOpenFileName(this, "Načítať vrstvy", "", "JSON (*.json)");
    if (!path.isEmpty())
    {
        if (vW->loadFromJson(path)) //bool
        {
            QMessageBox::information(this, "Načítané", "Vrstvy boli načítané.");
            refreshLayerList(); //naplním zoznam v UI
            vW->drawAllLayers(); //nakreslím
        } else {
            QMessageBox::warning(this, "Chyba", "Načítanie zlyhalo.");
        }
    }
}

//testy, pomoc pri refreshy
void ImageViewer::on_pushButtonRedrawLayers_clicked()
{
    vW->drawAllLayers();
    refreshLayerList();
}


void ImageViewer::refreshLayerList()
{
    ui->listWidgetLayers->clear(); // Vymažeme celý zoznam vrstiev vo Widgete

    QVector<Shape>& vrstvy = vW->getLayers(); //získam celé pole vrstiev Shapes

    for (int i = 0; i < vrstvy.size(); i++)
    {
        Shape shape = vrstvy[i];  // Zoberieme jednu vrstvu

        QString typText;

        // Typ útvaru ako text
        if (shape.type == 0) {
            typText = "Čiara";
        } else if (shape.type == 1) {
            typText = "Obdĺžnik";
        } else if (shape.type == 2) {
            typText = "Polygón";
        } else if (shape.type == 3) {
            typText = "Kružnica";
        } else if (shape.type == 4) {
            typText = "Bezierova krivka";
        } else {
            typText = "Neznámy";
        }

        // Vyplnenie ako text
        QString vyplnenie = shape.filled ? "filled" : "not filled";

        /*
        // Zložený popis
        QString popis = typText + " (" +
                        QString::number(shape.points.size()) + " bodov), Z=" +
                        QString::number(shape.zIndex) +
                        ", " + vyplnenie;
    */
        // index z pola
        QString popis = QString("[%1] ").arg(i) + typText + " (" +
                        QString::number(shape.points.size()) + " bodov), Z=" +
                        QString::number(shape.zIndex) +
                        ", " + vyplnenie;
        // Pridanie do zoznamu

        //dynamicky, lebo to chcem meniť počas behu programu
        //smerník na objekt QListWidgetItem
        QListWidgetItem* polozka = new QListWidgetItem(popis);

        //dodatočné dáta
        //polozka->setData(Qt::UserRole, i); // uložíme index vrstvy, index v poli layers

        polozka->setForeground(shape.borderColor);
        ui->listWidgetLayers->addItem(polozka);
    }
}



//ktoré je vybrané, s ktorým pracujem
void ImageViewer::on_listWidgetLayers_currentRowChanged(int row)
{
    if (row == -1) {
        qDebug() << "Klik mimo listu – výber ponechaný";
        return;
    }

    //vráti smerník na QLIstWidgetItem na danom indexe
    //item row vracia smerník
    QListWidgetItem* item = ui->listWidgetLayers->item(row);
    QString text = item->text();  // napr. "[3] Kružnica ..."

    QStringList casti = text.split(' '); //rozdelím podľa medzier, do pola
    if (casti.isEmpty()) return;

    QString indexText = casti[0];      // napr. "[3]" //prvá časť
    indexText.remove('[').remove(']'); // teraz "3"
    int index = indexText.toInt();

    if (index >= 0 && index < vW->getLayers().size()) {
        selectedLayerIndex = index;
        lockedLayerIndex = index;
    }
}







void ImageViewer::on_comboBoxInterpolation_currentIndexChanged(int index)
{
    vW->setInterpolationType(index);
    vW->update();
}



//plnenie Polygonu/trojuholníka
//zmena globálnej premennej
void ImageViewer::on_checkBoxFilled_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::Checked) //bool
    {
        isFilled = true;
    } else {
        isFilled = false;
    }

    vW->setisFilled(isFilled);
}



void ImageViewer::on_pushButtonDeleteLayer_clicked()
{
    qDebug() << "Ahoj";
    if (selectedLayerIndex < 0 || selectedLayerIndex >= vW->getLayers().size()) {
        QMessageBox::warning(this, "Žiadna vrstva", "Nie je vybratá žiadna vrstva.");
        return;
    }

    // Odstránim vrstvu z poľa vrstiev podľa indexu
    vW->getLayers().removeAt(selectedLayerIndex);

    // Vynulujeme výber (už nič nie je vybraté)
    selectedLayerIndex = -1;

    // Prekreslím všetky vrstvy, ktoré ostali
    vW->drawAllLayers();
    refreshLayerList(); //zmena indexov v poli, treba zmeniť, v UI
}



void ImageViewer::on_pushButtonLayerUp_clicked()
{
    // Ak nie je vybraná žiadna vrstva, nič nerobíme
    int pocetVrstiev = vW->getLayers().size();
    if (selectedLayerIndex < 0 || selectedLayerIndex >= pocetVrstiev)
        return;


    vW->moveLayerUp(selectedLayerIndex);
    vW->drawAllLayers();
    refreshLayerList();
}


void ImageViewer::on_pushButtonLayerDown_clicked()
{
    // či je platný index vybratej vrstvy
    int pocetVrstiev = vW->getLayers().size();
    if (selectedLayerIndex < 0 || selectedLayerIndex >= pocetVrstiev)
        return;


    vW->moveLayerDown(selectedLayerIndex);
    // Prekreslim a obnovit zoznam vrstiev
    vW->drawAllLayers();
    refreshLayerList();
}

void ImageViewer::on_pushButtonSetBorderColor_clicked()
{
    if (lockedLayerIndex < 0 || lockedLayerIndex >= vW->getLayers().size()) return;

    QColor color = QColorDialog::getColor(Qt::white, this, "Vyber farbu hrany");
    if (!color.isValid()) return;

    //mením originál
    QVector<Shape>& vrstvy = vW->getLayers();
    // Výber konkrétnej vrstvy podľa indexu
    Shape& vybranaVrstva = vrstvy[lockedLayerIndex];

    // zmena jej farby okraja
    vybranaVrstva.borderColor = color;


    vW->drawAllLayers();
}

void ImageViewer::on_pushButtonSetFillColor_clicked()
{
    if (lockedLayerIndex < 0 || lockedLayerIndex >= vW->getLayers().size()) return;

    QColor color = QColorDialog::getColor(Qt::white, this, "Vyber farbu výplne");
    if (!color.isValid()) return;

    QVector<Shape>& vrstvy = vW->getLayers();
    // Výber konkrétnej vrstvy podľa indexu
    Shape& vybranaVrstva = vrstvy[lockedLayerIndex];


    vybranaVrstva.fillColor = color; //prestavím farbu
    vW->drawAllLayers();
}





void ImageViewer::on_pushButtonChangeFillinfo_clicked()
{
    if (lockedLayerIndex < 0 || lockedLayerIndex >= vW->getLayers().size())
        return;

    QVector<Shape>& vrstvy = vW->getLayers();
    Shape& vrstva = vrstvy[lockedLayerIndex];

    // Len uzavreté útvary: Polygon alebo Rectangle
    if (vrstva.type == Polygon || vrstva.type == Rectangle || vrstva.type == Circle)
    {
        vrstva.filled = !vrstva.filled; // prepnúť stav
        vW->drawAllLayers();
        refreshLayerList();
    }

    else
    {
        QMessageBox::information(this, "Nevyplniteľné", "Tento útvar nepodporuje výplň.");
    }
}

