#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QImage>
#include "ColourRangeTable.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_refreshBtn_clicked();

	void on_saveJsonBtn_clicked();

	void on_loadJsonBtn_clicked();

	void on_saveImgBtn_clicked();

	void on_vanillaPaletteBtn_clicked();

	void on_gradientBtn_clicked();

private:
	Ui::MainWindow *ui;
	ColourRangeTable colourtable;
	QImage image;
};
#endif // MAINWINDOW_HPP
