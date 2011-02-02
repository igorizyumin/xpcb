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

#include <QSettings>
#include <QVBoxLayout>
#include <QShortcut>
#include "LayerWidget.h"
#include "global.h"
#include "Log.h"

LayerWidget::LayerWidget(QWidget* parent)
	: QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	this->setLayout(layout);
	mActiveLayer = LAY_TOP_COPPER;
	setNumLayers(16);
	setActive(LAY_TOP_COPPER);
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(&mMapper, SIGNAL(mapped(int)), this, SLOT(onKeyboardShortcut(int)));
}

void LayerWidget::rebuild()
{
	// delete any existing objects
	foreach(QShortcut* s, mShortcuts)
		delete s;
	mShortcuts.clear();
	foreach(QCheckBox* b, mCheckboxes)
		delete b;
	mCheckboxes.clear();
	for(int i = 0; i <= (int)LAY_TOP_COPPER; i++)
	{
		addLayer((PCBLAYER)i);
	}
	for(int i = 0; i < mNumLayers-2; i++)
	{
		addLayer((PCBLAYER)((int)LAY_INNER1+i));
	}
	addLayer(LAY_BOTTOM_COPPER);
	setActive(LAY_TOP_COPPER);
}

// returns the list index for a layer
int LayerWidget::mapLayer(PCBLAYER l) const
{
	int out = (int) l;
	if (out <= (int)LAY_TOP_COPPER)
		return out;
	if (out == (int) LAY_BOTTOM_COPPER)
		return out + mNumLayers - 2;
	else
		return out - 1;
}

QString getStylesheet(QColor color, bool active = false)
{
	return QString("QCheckBox { font-weight: %3; }\nQCheckBox:disabled { color: palette(text); }\nQCheckBox::indicator {width: 12px; height: 12px; border: %2 solid black; }\nQCheckBox::indicator::checked { background-color: %1; }\nQCheckBox::indicator::unchecked { background-image: url(:bg-disabled.png); }\n")
			.arg(color.name()).arg(active? "2px" : "1px").arg(active?"bold":"normal");
}

QCheckBox* makeCheckbox(QString name, QColor color)
{
	QCheckBox *cb = new QCheckBox(name);
	cb->setChecked(true);
	cb->setStyleSheet(getStylesheet(color));
	cb->setFocusPolicy(Qt::NoFocus);
	return cb;
}

void LayerWidget::addLayer(PCBLAYER l)
{
	const char keys[17] = "12345678QWERTYUI";

	QSettings s;
	if ((int)l >=0 && (int)l < MAX_LAYERS)
	{
		// get layer color
		QColor col = s.value(QString("colors/%1").arg(layer_str[(int)l])).value<QColor>();
		QString label = layer_str[(int)l];
		// add keyboard shortcut if necessary
		if (l >= LAY_TOP_COPPER)
		{
			int ind = mapLayer(l);
			ind -= (int)LAY_TOP_COPPER;
			label.append(QString(" [%1]").arg(keys[ind]));
			QShortcut* sc = new QShortcut(keys[ind], this);
			mShortcuts.append(sc);
			connect(sc, SIGNAL(activated()), &mMapper, SLOT(map()));
			mMapper.setMapping(sc, (int)l);
		}
		// make new checkbox
		QCheckBox* cb = makeCheckbox(label, col);
		connect(cb, SIGNAL(stateChanged(int)), this, SIGNAL(layerVisibilityChanged()));
		if (l == LAY_SELECTION || l == LAY_BACKGND)
			cb->setEnabled(false);

		// append checkbox and set ourselves as parent
		mCheckboxes.append(cb);
		this->layout()->addWidget(cb);
	}
}

void LayerWidget::setActive(PCBLAYER l)
{
	QSettings s;
	QColor prev = s.value(QString("colors/%1").arg(layer_str[(int)mActiveLayer])).value<QColor>();
	QColor next = s.value(QString("colors/%1").arg(layer_str[(int)l])).value<QColor>();

	// unset active for current
	mCheckboxes[mapLayer(mActiveLayer)]->setStyleSheet(getStylesheet(prev, false));
	mCheckboxes[mapLayer(l)]->setStyleSheet(getStylesheet(next, true));
	mActiveLayer = l;
	emit currLayerChanged(mActiveLayer);
}

bool LayerWidget::isLayerVisible(PCBLAYER l) const
{
	int ind = mapLayer(l);
	if (ind >= mCheckboxes.size()) return false;
	else return mCheckboxes[ind]->checkState();
}

void LayerWidget::onKeyboardShortcut(int layer)
{
	setActive((PCBLAYER)layer);
}
