/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
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

	bool isLayerVisible(const Layer& l) const;
	const Layer& activeLayer() const;

signals:
	void layerVisibilityChanged();
	void currLayerChanged(const Layer& layer);

public slots:
	void layersChanged(QList<Layer> layers) { rebuild(layers); }

private slots:
	void onKeyboardShortcut(int layer);

private:
	void rebuild(QList<Layer> layers);
	void addLayer(const Layer& l);
	void setActive(const Layer& l);

	Layer mActiveLayer;

	QSignalMapper mMapper;
	QList<Layer> mLayers;
	QList<QCheckBox*> mCheckboxes;
	QList<QShortcut*> mShortcuts;
	QSpacerItem* mSpacer;

};

#endif // LAYERWIDGET_H
