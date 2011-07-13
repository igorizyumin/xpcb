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

#include "SegmentLayerDialog.h"
#include "Controller.h"
#include "Document.h"

SegmentLayerDialog::SegmentLayerDialog(QWidget *parent, Controller* ctrl) :
	QDialog(parent), mCtrl(ctrl)
{
    setupUi(this);
	populateLayers();
}

SegmentLayerDialog::ApplyToObj SegmentLayerDialog::applyTo() const
{
	if (this->segButton->isChecked())
		return SEGMENT;
	else if (this->traceButton->isChecked())
		return TRACE;
	else
		return NET;
}

void SegmentLayerDialog::init(const Segment *seg)
{
	layerBox->setCurrentIndex(mLayers.indexOf(seg->layer()));
}

void SegmentLayerDialog::populateLayers()
{
	mLayers = mCtrl->doc()->layerList(Document::ListOrder, Document::Copper);

	foreach(const Layer& l, mLayers)
	{
		layerBox->addItem(l.name());
	}
}

Layer SegmentLayerDialog::layer() const
{
	return mLayers[layerBox->currentIndex()];
}
