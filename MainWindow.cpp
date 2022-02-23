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

enum WantedCorner {
	NONE,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};
struct PointDifference {
	const ColourDefinition* def;
	double xdiff;
	double ydiff;
};

static const ColourDefinition* getCloser(double x, double y, const ColourDefinition* a, const ColourDefinition* b) {
	const double dst1 = distance(a,x,y);
	const double dst2 = distance(b,x,y);
	if(dst2 >= dst1) return a;
	else return b;
}
static const ColourDefinition* getCloser(double x, double y, const ColourDefinition* a, const ColourDefinition* b, bool alast, bool blast) {
	const double dst1 = distance(a,alast,x,y);
	const double dst2 = distance(b,blast,x,y);
	if(dst2 >= dst1) return a;
	else return b;
}
static const PointDifference* getCloser(double x, double y, const PointDifference* a, const PointDifference* b) {
	const double dst1 = distance(a->def,x,y);
	const double dst2 = distance(b->def,x,y);
	if(dst2 >= dst1) return a;
	else return b;
}
static const PointDifference* getCloser(double x, double y, const PointDifference* a, const PointDifference* b, bool alast, bool blast) {
	const double dst1 = distance(a->def,alast,x,y);
	const double dst2 = distance(b->def,blast,x,y);
	if(dst2 >= dst1) return a;
	else return b;
}

static QColor getGradientColour(double x, double y, const ColourDefinition* defs, int defl) {
	QVector<PointDifference> diffs(defl);
	for(int i = 0; i < defl; ++i) {
		diffs[i].def = &defs[i];
		diffs[i].xdiff = defs[i].x2(i == (defl-1)) - x;
		diffs[i].ydiff = defs[i].y2(i == (defl-1)) - y;
	}
	QVector<PointDifference*> diffsTopLeft;
	QVector<PointDifference*> diffsTopRight;
	QVector<PointDifference*> diffsBottomLeft;
	QVector<PointDifference*> diffsBottomRight;
	diffsTopLeft.reserve(diffs.size());
	diffsTopRight.reserve(diffs.size());
	diffsBottomLeft.reserve(diffs.size());
	diffsBottomRight.reserve(diffs.size());
	for(int i = 0; i < diffs.size(); ++i) {
		// For qualify top-left
		if(diffs[i].ydiff <= 0.0 && diffs[i].xdiff <= 0.0) {
			diffsTopLeft.push_back(&diffs[i]);
		}
		// For qualify top-right
		else if(diffs[i].ydiff <= 0.0 && diffs[i].xdiff >= 0.0) {
			diffsTopRight.push_back(&diffs[i]);
		}
		// For qualify bottom-left
		if(diffs[i].ydiff >= 0.0 && diffs[i].xdiff <= 0.0) {
			diffsBottomLeft.push_back(&diffs[i]);
		}
		// For qualify bottom-right
		else if(diffs[i].ydiff >= 0.0 && diffs[i].xdiff >= 0.0) {
			diffsBottomRight.push_back(&diffs[i]);
		}
	}
	if(diffsTopLeft.isEmpty()) diffsTopLeft = diffsTopRight;
	else if(diffsTopRight.isEmpty()) diffsTopRight = diffsTopLeft;
	if(diffsBottomLeft.isEmpty()) diffsBottomLeft = diffsBottomRight;
	else if(diffsBottomRight.isEmpty()) diffsBottomRight = diffsBottomLeft;

	const PointDifference* closestTopLeft = diffsTopLeft[0];
	const PointDifference* closestTopRight = diffsTopRight[0];
	const PointDifference* closestBottomLeft = diffsBottomLeft[0];
	const PointDifference* closestBottomRight = diffsBottomRight[0];
	for(int i = 0; i < diffsTopLeft.size(); ++i) {
		closestTopLeft = getCloser(x,y,closestTopLeft,diffsTopLeft[i]);
	}
	for(int i = 0; i < diffsTopRight.size(); ++i) {
		closestTopRight = getCloser(x,y,closestTopRight,diffsTopRight[i]);
	}
	for(int i = 0; i < diffsBottomLeft.size(); ++i) {
		closestBottomLeft = getCloser(x,y,closestBottomLeft,diffsBottomLeft[i]);
	}
	for(int i = 0; i < diffsBottomRight.size(); ++i) {
		closestBottomRight = getCloser(x,y,closestBottomRight,diffsBottomRight[i]);
	}

	const double wTL = dist(closestTopLeft->def->xmin, x) * dist(closestTopLeft->def->ymin, y);
	const double wTR = dist(closestTopRight->def->xmax, x) * dist(closestTopRight->def->ymin, y);
	const double wBL = dist(closestBottomLeft->def->xmin, x) * dist(closestBottomLeft->def->ymax, y);
	const double wBR = dist(closestBottomRight->def->xmax, x) * dist(closestBottomRight->def->ymax, y);

	//std::cout << "Unified weight: " << wTL + wTR + wBL + wBR << std::endl;

	QColor tmp;
	tmp.setRgbF(
				// R
				(wTL * closestTopLeft->def->colour.redF()) + (wTR * closestTopRight->def->colour.redF())
				+ (wBL * closestBottomLeft->def->colour.redF()) + (wBR * closestTopRight->def->colour.redF()),
				// G
				(wTL * closestTopLeft->def->colour.greenF()) + (wTR * closestTopRight->def->colour.greenF())
				+ (wBL * closestBottomLeft->def->colour.greenF()) + (wBR * closestTopRight->def->colour.greenF()),
				// B
				(wTL * closestTopLeft->def->colour.blueF()) + (wTR * closestTopRight->def->colour.blueF())
				+ (wBL * closestBottomLeft->def->colour.blueF()) + (wBR * closestTopRight->def->colour.blueF())
				);
	return tmp;
}

void MainWindow::on_gradientBtn_clicked()
{
	if(!colourtable.rowCount()) return;
	for(int y = 0; y < image.height(); ++y ) {
		QRgb* scanline = reinterpret_cast<QRgb*>(image.scanLine(y));
		for(int x = 0; x < image.width() ; ++x) {
			const double fx = static_cast<double>(x) / static_cast<double>(image.width());
			const double fy = static_cast<double>(y) / static_cast<double>(image.height());
			scanline[x] = getGradientColour(fx,fy,colourtable.getColours().data(),colourtable.getColours().size()).rgb();
		}
	}
	ui->label->setPixmap(QPixmap::fromImage(image));
}

