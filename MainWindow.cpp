#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>

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

