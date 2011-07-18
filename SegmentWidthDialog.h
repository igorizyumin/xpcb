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

#ifndef SEGMENTWIDTHDIALOG_H
#define SEGMENTWIDTHDIALOG_H

#include "ui_SegmentWidthDialog.h"
#include "global.h"
#include "Trace.h"

class SegmentWidthDialog : public QDialog, private Ui::SegmentWidthDialog
{
    Q_OBJECT

public:
	enum ApplyToObj { SEGMENT, TRACE, NET };

	explicit SegmentWidthDialog(QWidget *parent);

	void init(const Segment* seg = 0);

	Dimension width() const { return widthBox->value(); }

	ApplyToObj applyTo() const;

};

#endif // SEGMENTWIDTHDIALOG_H
