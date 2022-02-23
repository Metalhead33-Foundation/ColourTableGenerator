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

	inline constexpr double x() const { return (xmin + xmax) / 2.0; }
	inline constexpr double y() const { return (ymin + ymax) / 2.0; }
	inline constexpr double x2(bool isLast) const { return isLast ? xmax : xmin; }
	inline constexpr double y2(bool isLast) const { return isLast ? ymax : ymin; }

};

#endif // COLOURDEFINITION_HPP
