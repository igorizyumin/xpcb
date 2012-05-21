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

#ifndef EDITPART_H
#define EDITPART_H

#include <QPainter>
#include "global.h"

/// Abstract class that implements the controller of each PCB element.
/// Links together model and view.
class EditPart : public QObject
{
    Q_OBJECT
public:
    explicit EditPart(QObject *parent = 0);

	/// Draws the object using the provided QPainter.  This function is
	/// called multiple times during a single redraw operation, once for each layer.
	/// \param painter the painter to use
	/// \param layer the PCB layer to draw
	virtual void draw(QPainter *painter, const Layer& layer) const = 0;

	/// Returns the object's bounding box
	virtual QRect bbox() const = 0;

	/// Returns true if the object was hit (pt is less than the specified
	/// distance away from the object).
	virtual bool testHit(QPoint /* pt */, int /* distance */,
						 const Layer& /*l*/) const { return false; }


signals:

public slots:

};

#endif // EDITPART_H
