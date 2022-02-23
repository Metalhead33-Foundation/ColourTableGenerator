#ifndef COLOURDEFINITION_HPP
#define COLOURDEFINITION_HPP
#include <QString>
#include <QColor>
#include <QJsonObject>

struct ColourDefinition {
	QString label;
	QColor colour;
	double xmin, ymin, xmax, ymax;

	void fromJson(const QJsonObject& json);
	void toJson(QJsonObject& json) const;
	QJsonObject toJson(void) const;
};

#endif // COLOURDEFINITION_HPP
