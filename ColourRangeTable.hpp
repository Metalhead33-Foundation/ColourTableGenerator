#ifndef COLOURRANGETABLE_HPP
#define COLOURRANGETABLE_HPP

#include <QAbstractTableModel>
#include <QList>
#include "ColourDefinition.hpp"
#include <QJsonArray>
#include <functional>

class ColourRangeTable : public QAbstractTableModel
{
	Q_OBJECT
private:
	QVector<ColourDefinition> colours;
public:
	typedef std::function<void(ColourDefinition&)> ColourUser;
	typedef std::function<void(const ColourDefinition&)> ConstColourUser;

	explicit ColourRangeTable(QObject *parent = nullptr);

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	// Editable:
	bool setData(const QModelIndex &index, const QVariant &value,
				 int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	// Add data:
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	// Remove data:
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	void purge();

	void fromJson(const QJsonArray& json);
	void toJson(QJsonArray& json) const;
	QJsonArray toJson(void) const;
	void iterateAndUse(const ColourUser& colour_function);
	void iterateAndUse(const ConstColourUser& colour_function) const;

	const QVector<ColourDefinition>& getColours() const;

private:
};

#endif // COLOURRANGETABLE_HPP
