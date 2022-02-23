#include "ColourDefinition.hpp"

void ColourDefinition::fromJson(const QJsonObject& json)
{
	label = json[QStringLiteral("label")].toString();
	colour = json[QStringLiteral("colour")].toVariant().value<QColor>();
	xmin = std::clamp(json[QStringLiteral("xmin")].toDouble(),0.0, 1.0);
	ymin = std::clamp(json[QStringLiteral("ymin")].toDouble(),0.0, 1.0);
	xmax = std::clamp(json[QStringLiteral("xmax")].toDouble(),0.0, 1.0);
	ymax = std::clamp(json[QStringLiteral("ymax")].toDouble(),0.0, 1.0);
}

/*
	QString label;
	QColor colour;
	double xmin, ymin, xmax, ymax;
*/

void ColourDefinition::toJson(QJsonObject& json) const
{
	json[QStringLiteral("label")] = label;
	json[QStringLiteral("colour")] = QJsonValue::fromVariant(colour);
	json[QStringLiteral("xmin")] = xmin;
	json[QStringLiteral("ymin")] = ymin;
	json[QStringLiteral("xmax")] = xmax;
	json[QStringLiteral("ymax")] = ymax;
}

QJsonObject ColourDefinition::toJson() const
{
	QJsonObject tmp;
	toJson(tmp);
	return tmp;
}
