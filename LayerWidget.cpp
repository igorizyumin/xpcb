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
	: QWidget(parent), mSpacer(NULL)
{
	QVBoxLayout* layout = new QVBoxLayout();
	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	connect(&mMapper, SIGNAL(mapped(int)), this, SLOT(onKeyboardShortcut(int)));
}

void LayerWidget::rebuild(QList<Layer> layers)
{
	// delete any existing objects
	foreach(QShortcut* s, mShortcuts)
		delete s;
	mShortcuts.clear();
	foreach(QCheckBox* c, mCheckboxes)
		delete c;
	mCheckboxes.clear();
	mLayers.clear();
	this->layout()->removeItem(mSpacer);
	delete mSpacer;
	mSpacer = NULL;
	bool setact = true;
	Layer active;
	foreach(const Layer& l, layers)
	{
		addLayer(l);
		if (setact && l.isCopper())
		{
			active = l;
			setact = false;
		}
	}

	mSpacer = new QSpacerItem(1, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	this->layout()->addItem(mSpacer);
	setActive(active);
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

void LayerWidget::addLayer(const Layer& l)
{
	const int maxSc = 17;
	const char keys[maxSc] = "12345678QWERTYUI";

	mLayers.append(l);

	QString label = l.name();

	// add keyboard shortcut if necessary
	int keyInd = mShortcuts.size();
	if (l.isCopper() && keyInd < maxSc)
	{
		label.append(QString(" [%1]").arg(keys[keyInd]));
		QShortcut* sc = new QShortcut(keys[keyInd], this);
		mShortcuts.append(sc);
		connect(sc, SIGNAL(activated()), &mMapper, SLOT(map()));
		mMapper.setMapping(sc, mLayers.size()-1);
	}
	// make new checkbox
	QCheckBox* cb = makeCheckbox(label, l.color());
	connect(cb, SIGNAL(stateChanged(int)), this, SIGNAL(layerVisibilityChanged()));

	// append checkbox and set ourselves as parent
	mCheckboxes.append(cb);
	this->layout()->addWidget(cb);
}

void LayerWidget::setActive(const Layer& l)
{
	if (!mLayers.contains(l))
		return;
	QColor prev = mActiveLayer.color();
	QColor next = l.color();
	int prevind = mLayers.indexOf(mActiveLayer);
	int newind = mLayers.indexOf(l);

	// unset active for current
	if (prevind != -1)
		mCheckboxes[prevind]->setStyleSheet(getStylesheet(prev, false));
	// set active for new
	if (newind != -1)
		mCheckboxes[newind]->setStyleSheet(getStylesheet(next, true));
	mActiveLayer = l;
	emit currLayerChanged(mActiveLayer);
}

bool LayerWidget::isLayerVisible(const Layer& l) const
{
	if (!mLayers.contains(l))
		return false;
	return mCheckboxes[mLayers.indexOf(l)]->checkState();
}

const Layer& LayerWidget::activeLayer() const
{
	return mActiveLayer;
}

void LayerWidget::onKeyboardShortcut(int layer)
{
	setActive(mLayers[layer]);
}
