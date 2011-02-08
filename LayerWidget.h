/*
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QSignalMapper>
#include <QShortcut>
#include <QSpacerItem>
#include "global.h"

class LayerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit LayerWidget(QWidget* parent = 0);

	void setNumLayers(int numLayers) { mNumLayers = numLayers; rebuild(); }
	bool isLayerVisible(PCBLAYER l) const;
	PCBLAYER activeLayer() const;

signals:
	void layerVisibilityChanged();
	void currLayerChanged(PCBLAYER layer);

private slots:
	void onKeyboardShortcut(int layer);

private:
	void rebuild();
	void addLayer(PCBLAYER l);
	void setActive(PCBLAYER l);
	int mapLayer(PCBLAYER l) const;

	PCBLAYER mActiveLayer;
	int mNumLayers;

	QSignalMapper mMapper;
	QList<QCheckBox*> mCheckboxes;
	QList<QShortcut*> mShortcuts;
	QSpacerItem* mSpacer;

};

#endif // LAYERWIDGET_H
