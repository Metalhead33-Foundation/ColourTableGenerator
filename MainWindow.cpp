#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <cmath>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow), image(256,256,QImage::Format_ARGB32)
{
	ui->setupUi(this);
	ui->tableView->setModel(&colourtable);
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_refreshBtn_clicked()
{
	colourtable.iterateAndUse( [this](const ColourDefinition& def){
		QRgb rgb = def.colour.rgb();
		const int xmin = static_cast<int>(def.xmin * static_cast<double>(image.width()));
		const int xmax = static_cast<int>(def.xmax * static_cast<double>(image.width()));
		const int ymin = static_cast<int>(def.ymin * static_cast<double>(image.height()));
		const int ymax = static_cast<int>(def.ymax * static_cast<double>(image.height()));
		for(int y = ymin; y < ymax; ++y ) {
			QRgb* scanline = reinterpret_cast<QRgb*>(image.scanLine(y));
			for(int x = xmin; x < xmax ; ++x) {
				scanline[x] = rgb;
			}
		}
	} );
	ui->label->setPixmap(QPixmap::fromImage(image));
}


void MainWindow::on_saveJsonBtn_clicked()
{
	QString path = QFileDialog::getSaveFileName(this,tr("Save JSON definition"),QString(),tr("JSON (*.json)"));
	if(path.isEmpty()) return;
	QFile file(path);
	if(file.open(QFile::WriteOnly)) {
		QJsonDocument document;
		document.setArray(colourtable.toJson());
		file.write(document.toJson());
		file.flush();
		file.close();
	} else {
		QMessageBox::critical(this,tr("Error!"),tr("Failed to open \"%1\" for writing!").arg(path));
	}
}


void MainWindow::on_loadJsonBtn_clicked()
{
	QString path = QFileDialog::getOpenFileName(this,tr("Load JSON definition"),QString(),tr("JSON (*.json)"));
	if(path.isEmpty()) return;
	QFile file(path);
	if(file.open(QFile::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		file.close();
		colourtable.fromJson(document.array());
	} else {
		QMessageBox::critical(this,tr("Error!"),tr("Failed to open \"%1\" for reading!").arg(path));
	}
}


void MainWindow::on_saveImgBtn_clicked()
{
	QString path = QFileDialog::getSaveFileName(this,tr("Save image file"),QString(),tr("PNG files (*.png)"));
	if(path.isEmpty()) return;
	if(!image.save(path)) {
		QMessageBox::critical(this,tr("Error!"),tr("Failed to open \"%1\" for writing!").arg(path));
	}
}


void MainWindow::on_vanillaPaletteBtn_clicked()
{
	QString path = QFileDialog::getOpenFileName(this,tr("Load JSON definition"),tr("PNG files (*.png)"));
	if(path.isEmpty()) return;
	QImage img(path);
	if(img.isNull()) return;
	colourtable.iterateAndUse( [&img](ColourDefinition& def){
		const int x = static_cast<int>(((def.xmin + def.xmax) / 2.0)  * static_cast<double>(img.width()));
		const int y = static_cast<int>(((def.ymin + def.ymax) / 2.0) * static_cast<double>(img.width()));
		def.colour = img.pixelColor(x,y);
	} );
	on_refreshBtn_clicked();
}

static double dist(double a, double b) {
	return std::max(a,b) - std::min(a,b);
}
static double distance(const ColourDefinition* a, double x, double y) {
	return std::sqrt( std::pow(a->x() - x,2.0) + std::pow(a->y() - y,2.0));
}
static double distance(const ColourDefinition* a, bool alast, double x, double y) {
	return std::sqrt( std::pow(a->x2(alast) - x,2.0) + std::pow(a->y2(alast) - y,2.0));
}

struct Point {
	QColor colour;
	double x,y;
};

struct Cube {
	int topLeft, topRight, bottomLeft, bottomRight;
};

int addPoint(QVector<Point>& points, double x, double y, const QColor& colour) {
	int index = 0;
	for(auto& it : points) {
		if(it.x == x && it.y == y) {
			const qreal nr = (0.5 * it.colour.redF()) + (0.5 * colour.redF());
			const qreal ng = (0.5 * it.colour.greenF()) + (0.5 * colour.greenF());
			const qreal nb = (0.5 * it.colour.blueF()) + (0.5 * colour.blueF());
			it.colour.setRgbF(nr,ng,nb);
			return index;
		}
		++index;
	}
	points.push_back( { colour, x, y } );
	return index;
}

void MainWindow::on_gradientBtn_clicked()
{
	if(!colourtable.rowCount()) return;
	QVector<Point> points;
	QVector<Cube> quads;
	colourtable.iterateAndUse( [&points,&quads](const ColourDefinition& cdef) {
		int topLeft = addPoint(points,cdef.xmin,cdef.ymin,cdef.colour);
		int topRight = addPoint(points,cdef.xmax,cdef.ymin,cdef.colour);
		int bottomLeft = addPoint(points,cdef.xmin,cdef.ymax,cdef.colour);
		int bottomRight = addPoint(points,cdef.xmax,cdef.ymax,cdef.colour);
		quads.push_back( { topLeft, topRight, bottomLeft, bottomRight } );
	});
	for(const auto& it : quads) {
		const auto& topLeft = points[it.topLeft];
		const auto& topRight = points[it.topRight];
		const auto& bottomLeft = points[it.bottomLeft];
		const auto& bottomRight = points[it.bottomRight];
		const int xmin = static_cast<int>( topLeft.x * static_cast<double>(image.width()));
		const int xmax = static_cast<int>( topRight.x * static_cast<double>(image.width()));
		const int ymin = static_cast<int>( topLeft.y * static_cast<double>(image.height()));
		const int ymax = static_cast<int>( bottomLeft.y * static_cast<double>(image.height()));

		const double scaleY = bottomLeft.y - topLeft.y;
		const double scaleX = bottomRight.x - bottomLeft.x;
		for(int y = ymin; y < ymax; ++y ) {
			QRgb* scanline = reinterpret_cast<QRgb*>(image.scanLine(y));
			for(int x = xmin; x < xmax ; ++x) {
				const double fx = static_cast<double>(x) / static_cast<double>(image.width());
				const double fy = static_cast<double>(y) / static_cast<double>(image.height());

				/*const double wtl = ((fx - topLeft.x) / scaleX) * ((fy - topLeft.y) / scaleY);
				const double wtr = ((topRight.x - fx ) / scaleX) * ((fy - topRight.y) / scaleY);
				const double wbl = ((fx - bottomLeft.x) / scaleX) * ((bottomLeft.y - fy) / scaleY);
				const double wbr = ((bottomRight.x - fx ) / scaleX) * ((bottomRight.y - fy) / scaleY);*/

				const double wtl = ((topLeft.x - fx) / scaleX) * ((topLeft.y - fy) / scaleY);
				const double wtr = ((fx -topRight.x) / scaleX) * ((topRight.y - fy) / scaleY);
				const double wbl = ((bottomLeft.x - fx) / scaleX) * ((fy - bottomLeft.y) / scaleY);
				const double wbr = ((fx - bottomRight.x) / scaleX) * ((fy - bottomRight.y) / scaleY);

				QColor tmpColour;

				tmpColour.setRgbF(
							(wbr * topLeft.colour.redF()) + (wbl * topRight.colour.redF()) + (wtr * bottomLeft.colour.redF()) + (wtl * bottomRight.colour.redF()),
							(wbr * topLeft.colour.greenF()) + (wbl * topRight.colour.greenF()) + (wtr * bottomLeft.colour.greenF()) + (wtl * bottomRight.colour.greenF()),
							(wbr * topLeft.colour.blueF()) + (wbl * topRight.colour.blueF()) + (wtr * bottomLeft.colour.blueF()) + (wtl * bottomRight.colour.blueF())
						/*(wtl * topLeft.colour.redF()) + (wtr * topRight.colour.redF()) + (wbl * bottomLeft.colour.redF()) + (wbr * bottomRight.colour.redF()),
						(wtl * topLeft.colour.greenF()) + (wtr * topRight.colour.greenF()) + (wbl * bottomLeft.colour.greenF()) + (wbr * bottomRight.colour.greenF()),
						(wtl * topLeft.colour.blueF()) + (wtr * topRight.colour.blueF()) + (wbl * bottomLeft.colour.blueF()) + (wbr * bottomRight.colour.blueF())*/
							, 1.0
				);

				scanline[x] = tmpColour.rgb();
			}
		}
	}

	ui->label->setPixmap(QPixmap::fromImage(image));
}

