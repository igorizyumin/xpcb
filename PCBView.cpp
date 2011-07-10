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

#include "global.h"
#include "PCBView.h"
#include "Document.h"
#include "Log.h"
#include "Controller.h"
#include <QSize>
#include <QPainter>
#include <QMouseEvent>
#include <QSettings>

PCBView::PCBView(QWidget *parent)
	: QWidget(parent), mCtrl(NULL), mWheelAngle(0)
{
	// initialize transform
	// 100 pixels = 1 inch
	mTransform.translate(0, 300);
	mTransform.scale(100.0/XPcb::inchToPcb(1), -100.0/XPcb::inchToPcb(1));
	mVisibleGrid = XPcb::inchToPcb(0.1);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
}

PCBView::~PCBView()
{

}

QSize PCBView::sizeHint() const
{
	return QSize(800,600);
}

void PCBView::setCtrl(Controller *ctrl)
{
	mCtrl = ctrl;
}

void PCBView::visGridChanged(int grid)
{
	this->mVisibleGrid = grid;
	update();
}

void PCBView::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	// erase background
	painter.setBackground(QBrush(Layer::color(Layer::LAY_BACKGND)));
	painter.setClipping(true);
	painter.eraseRect(this->rect());
	// draw document
	if (mCtrl && mCtrl->docIsOpen())
	{
		QList<Layer> layers = mCtrl->doc()
				->layerList(PCBDoc::DrawPriorityOrder);
		// draw grid
		QPen pen(Layer::color(Layer::LAY_VISIBLE_GRID));
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		painter.setTransform(mTransform);
		painter.setPen(pen);
		if (mCtrl->isLayerVisible(Layer::LAY_VISIBLE_GRID))
		{
			drawOrigin(&painter);
			drawGrid(&painter);
		}
		// turn on AA
		painter.setRenderHint(QPainter::Antialiasing);
		// set transform
		QRect bb = mTransform.inverted().mapRect(e->rect());
		// figure out active layer
		Layer active = mCtrl->activeLayer();

		// draw the layers
		bool activeDrawn = false;
		QListIterator<Layer> i(layers);
		while(i.hasNext())
		{
			Q_ASSERT(painter.isActive());
			Layer curr;
			// find first copper layer to draw current active layer
			if (!activeDrawn && !i.peekNext().isCopper()
					&& i.hasPrevious() && i.peekPrevious().isCopper())
			{
				curr = active;
				activeDrawn = true;
			}
			else
			{
				curr = i.next();
				if (curr == active)
					continue; // will draw later
			}
			if (mCtrl->isLayerVisible(curr))
			{
				// set color / fill
				QPen p = painter.pen();
				p.setColor(curr.color());
				p.setWidth(0);
				painter.setPen(p);
				QBrush b = painter.brush();
				b.setColor(curr.color());
				if (curr.type() != Layer::LAY_SELECTION)
					b.setStyle(Qt::SolidPattern);
				else
					b.setStyle(Qt::NoBrush);
				painter.setBrush(b);
				// tell controller to draw it
				mCtrl->draw(&painter, bb, curr);
			}
		}
	}
	painter.end();
}

void PCBView::drawOrigin(QPainter *painter)
{
	// circle with 4 lines
	painter->drawEllipse(XPcb::inchToPcb(-0.05),XPcb::inchToPcb(-0.05),
						 XPcb::inchToPcb(0.1), XPcb::inchToPcb(0.1));
	painter->drawLine(XPcb::inchToPcb(0.05), 0, XPcb::inchToPcb(0.25), 0);
	painter->drawLine(0, XPcb::inchToPcb(0.05), 0, XPcb::inchToPcb(0.25));
	painter->drawLine(XPcb::inchToPcb(-0.05), 0, XPcb::inchToPcb(-0.25), 0);
	painter->drawLine(0, XPcb::inchToPcb(-0.05), 0, XPcb::inchToPcb(-0.25));
}

void PCBView::drawGrid(QPainter *painter)
{
//	painter->drawRect(QRect(QPoint(-PCB_BOUND, -PCB_BOUND), QPoint(PCB_BOUND, PCB_BOUND)));
	QRect viewport = mTransform.inverted().mapRect(this->rect());
	if (mTransform.map(QLine(QPoint(0,0), QPoint(mVisibleGrid, 0))).dx() <= 5)
		return;
	int startX = (viewport.x() / mVisibleGrid) * mVisibleGrid;
	int startY = (viewport.y() / mVisibleGrid) * mVisibleGrid;
	int endX = viewport.x() + viewport.width();
	int endY = viewport.y() + viewport.height();
	for(int x = startX; x <= endX; x += mVisibleGrid)
		for(int y = startY; y <= endY; y += mVisibleGrid)
			painter->drawPoint(x, y);
}

void PCBView::mouseMoveEvent(QMouseEvent * event)
{
	mMousePos = event->pos();
	emit mouseMoved(mTransform.inverted().map(mMousePos));
}

void PCBView::mousePressEvent(QMouseEvent * event)
{
	event->ignore();
}

void PCBView::mouseReleaseEvent(QMouseEvent * event)
{
	event->ignore();
}

void PCBView::wheelEvent(QWheelEvent *event)
{
	mWheelAngle += event->delta();
	if (mWheelAngle >= 120)
	{
		// zoom in
		mWheelAngle -= 120;
		zoom(1.25, event->pos());
	}
	else if (mWheelAngle <= -120)
	{
		// zoom out
		mWheelAngle += 120;
		zoom(0.8, event->pos());
	}
}

void PCBView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)
	{
		recenter(mMousePos);
	}
	else
		event->ignore();
}

void PCBView::enterEvent(QEvent */*event*/)
{
	setFocus(Qt::MouseFocusReason);
}

void PCBView::leaveEvent(QEvent */*event*/)
{
	clearFocus();
}

/// Recenter view around pt (in screen coords by default)
void PCBView::recenter(QPoint pt, bool world)
{
	if (!mCtrl || !mCtrl->docIsOpen()) return;
	QPoint ctr = mTransform.inverted().map(this->rect().center());
	if (!world)
		pt = mTransform.inverted().map(pt);
	ctr -= pt;
	mTransform.translate(ctr.x(), ctr.y());
	// check if the viewport is out of bounds
	QRect bound = QRect(QPoint(-XPcb::PCB_BOUND, -XPcb::PCB_BOUND),
						QPoint(XPcb::PCB_BOUND, XPcb::PCB_BOUND));
	QRect vp = mTransform.inverted().mapRect(this->rect());
	if (!bound.contains(vp))
	{
		Log::instance().message(QString("(L: %1; R: %2; T: %3; B: %4)")
								.arg(vp.left()).arg(vp.right())
								.arg(vp.top()).arg(vp.bottom()));
		if (vp.left() < bound.left())
		{
			mTransform.translate(-(bound.left() - vp.left()), 0);
		}
		else if (vp.right() > bound.right())
		{
			mTransform.translate(-(bound.right() - vp.right()), 0);
		}
		if (vp.top() < bound.top())
		{
			mTransform.translate(0, -(bound.top() - vp.top()));
		}
		if (vp.bottom() > bound.bottom())
		{
			mTransform.translate(0, -(bound.bottom() - vp.bottom()));
		}

	}
	// move cursor to middle of the window
	QCursor::setPos(mapToGlobal(rect().center()));
	// force repaint
	update();
}

/// Pos is in screen coords
void PCBView::zoom(double factor, QPoint pos)
{
	if (!mCtrl || !mCtrl->docIsOpen()) return;
	recenter(pos);
	QPoint p = mTransform.inverted().map(rect().center());
	QTransform test = mTransform;
	test.scale(factor, factor);
	// check if the bounding rectangle does not enclose the viewport
	// refuse to zoom out (factor < 1) if that's the case
	if(!test.mapRect(QRect(QPoint(-XPcb::PCB_BOUND, -XPcb::PCB_BOUND),
								QPoint(XPcb::PCB_BOUND, XPcb::PCB_BOUND)))
			.contains(this->rect())	&& factor < 1 )
	{
		return;
	}
	mTransform = test;
	recenter(p, true);
}
