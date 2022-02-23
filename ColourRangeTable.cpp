#include "ColourRangeTable.hpp"
#include <algorithm>

ColourRangeTable::ColourRangeTable(QObject *parent)
	: QAbstractTableModel(parent)
{
}

QVariant ColourRangeTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	// FIXME: Implement me!
	switch (orientation) {
		case Qt::Vertical: { switch (role) {
			case Qt::DisplayRole: return section + 1;
			default: return QVariant();
		}
		break;
		}
			case Qt::Horizontal: { switch (role) {
			case Qt::DisplayRole: {
			switch (section) {
				case 0: return tr("Label");
				case 1: return tr("X-Min");
				case 2: return tr("X-Max");
				case 3: return tr("Y-Min");
				case 4: return tr("Y-Max");
				case 5: return tr("Colour");
				case 6: return tr("Delete");
				default: return QVariant();
			}
			}
			default: return QVariant();
		}
		break;
		}
		default: return QVariant();
	}
}

int ColourRangeTable::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return colours.size();
}

int ColourRangeTable::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return 6;
}

QVariant ColourRangeTable::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if(!(index.column() >= 0 && index.column() <= 6)) return QVariant();
	if(index.row() > colours.size()) return QVariant();
	switch (role) {
		case Qt::BackgroundColorRole: {
			switch (index.column()) {
				case 5: return colours[index.row()].colour;
				default: return QColor(Qt::transparent);
			}
		}
		case Qt::EditRole: {
			switch (index.column()) {
				case 0: return colours[index.row()].label;
				case 1: return colours[index.row()].xmin;
				case 2: return colours[index.row()].xmax;
				case 3: return colours[index.row()].ymin;
				case 4: return colours[index.row()].ymax;
				case 5: return colours[index.row()].colour;
				case 6: return QStringLiteral("    ");
				default: return QVariant();
			}
		}
		case Qt::DisplayRole: {
			switch (index.column()) {
				case 0: return colours[index.row()].label;
				case 1: return colours[index.row()].xmin;
				case 2: return colours[index.row()].xmax;
				case 3: return colours[index.row()].ymin;
				case 4: return colours[index.row()].ymax;
				case 5: return QStringLiteral("    ");
				case 6: return QStringLiteral("    ");
				default: return QVariant();
			}
		}
		default: return QVariant();
	}
}

bool ColourRangeTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role != Qt::EditRole) return false;
	if(!(index.column() >= 0 && index.column() <= 6)) return false;
	if(index.row() > colours.size()) return false;
	if (data(index, role) != value) {
		switch (index.column()) {
			case 0: colours[index.row()].label = value.toString(); break;
			case 1: colours[index.row()].xmin = std::clamp(value.toDouble(),0.0, 1.0); break;
			case 2: colours[index.row()].xmax = std::clamp(value.toDouble(),0.0, 1.0); break;
			case 3: colours[index.row()].ymin = std::clamp(value.toDouble(),0.0, 1.0); break;
			case 4: colours[index.row()].ymax = std::clamp(value.toDouble(),0.0, 1.0); break;
			case 5: colours[index.row()].colour = value.value<QColor>(); break;
			default: break;
		}
		emit dataChanged(index, index, QVector<int>() << role);
		return true;
	}
	return false;
}

Qt::ItemFlags ColourRangeTable::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable; // FIXME: Implement me!
}

bool ColourRangeTable::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row + count - 1);
	for(int i = 0; i < count; ++i) {
		colours.insert(row,ColourDefinition());
	}
	endInsertRows();
}

bool ColourRangeTable::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row + count - 1);
	for(int i = 0; i < count; ++i) {
		colours.removeAt(row);
	}
	endRemoveRows();
}

void ColourRangeTable::purge()
{
	if(!colours.size()) return;
	beginRemoveRows(QModelIndex(), 0, colours.size());
	colours.clear();
	endRemoveRows();
}

void ColourRangeTable::fromJson(const QJsonArray& json)
{
	purge();
	beginInsertRows(QModelIndex(), 0, json.size());
	for(const auto& it : json) {
		ColourDefinition tmp;
		tmp.fromJson(it.toObject());
		colours.push_back(tmp);
	}
	endInsertRows();
}

void ColourRangeTable::toJson(QJsonArray& json) const
{
	for(const auto& it : colours) {
		json.push_back(it.toJson());
	}
}

QJsonArray ColourRangeTable::toJson() const
{
	QJsonArray tmp;
	toJson(tmp);
	return tmp;
}

void ColourRangeTable::iterateAndUse(const ColourUser& colour_function)
{
	for(auto& it : colours) {
		colour_function(it);
	}
}

void ColourRangeTable::iterateAndUse(const ConstColourUser& colour_function) const
{
	for(const auto& it : colours) {
		colour_function(it);
	}
}
