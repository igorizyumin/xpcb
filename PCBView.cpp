#include "global.h"
#include "PCBView.h"
#include "PCBDoc.h"
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
	mTransform.scale(100.0/XPcb::IN2PCB(1), -100.0/XPcb::IN2PCB(1));
	mVisibleGrid = XPcb::IN2PCB(0.1);
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
	const int nLayers = 26;
	// layer draw order
	const XPcb::PCBLAYER priority[nLayers] = {
		XPcb::LAY_BOTTOM_COPPER,
		XPcb::LAY_INNER14,
		XPcb::LAY_INNER13,
		XPcb::LAY_INNER12,
		XPcb::LAY_INNER11,
		XPcb::LAY_INNER10,
		XPcb::LAY_INNER9,
		XPcb::LAY_INNER8,
		XPcb::LAY_INNER7,
		XPcb::LAY_INNER6,
		XPcb::LAY_INNER5,
		XPcb::LAY_INNER4,
		XPcb::LAY_INNER3,
		XPcb::LAY_INNER2,
		XPcb::LAY_INNER1,
		XPcb::LAY_TOP_COPPER,
		XPcb::LAY_CURR_ACTIVE,
		XPcb::LAY_HOLE,
		XPcb::LAY_SM_BOTTOM,
		XPcb::LAY_SM_TOP,
		XPcb::LAY_SILK_BOTTOM,
		XPcb::LAY_SILK_TOP,
		XPcb::LAY_RAT_LINE,
		XPcb::LAY_BOARD_OUTLINE,
		XPcb::LAY_DRC_ERROR,
		XPcb::LAY_SELECTION
	};

	QPainter painter(this);
	// erase background
	painter.setBackground(QBrush(layerColor(XPcb::LAY_BACKGND)));
	painter.setClipping(true);
	painter.eraseRect(this->rect());
	if (mCtrl && mCtrl->docIsOpen())
	{
		// draw grid
		QPen pen(layerColor(XPcb::LAY_VISIBLE_GRID));
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		painter.setTransform(mTransform);
		painter.setPen(pen);
		if (mCtrl->isLayerVisible(XPcb::LAY_VISIBLE_GRID))
		{
			drawOrigin(&painter);
			drawGrid(&painter);
		}
		// turn on AA
		painter.setRenderHint(QPainter::Antialiasing);
		QRect bb = mTransform.inverted().mapRect(e->rect());
		XPcb::PCBLAYER active = mCtrl->activeLayer();

		// draw the layers
		for(int i = 0; i < nLayers; i++)
		{
			XPcb::PCBLAYER l = priority[i];
			if (l == XPcb::LAY_CURR_ACTIVE) // placeholder
				l = active;
			else if (l == active)
				continue; // already drawn
			if (mCtrl->isLayerVisible(l))
			{
				// set color / fill
				QPen p = painter.pen();
				p.setColor(layerColor((XPcb::PCBLAYER)l));
				p.setWidth(0);
				painter.setPen(p);
				QBrush b = painter.brush();
				b.setColor(layerColor(l));
				if (l != XPcb::LAY_SELECTION)
					b.setStyle(Qt::SolidPattern);
				else
					b.setStyle(Qt::NoBrush);
				painter.setBrush(b);
				// tell controller to draw it
				mCtrl->draw(&painter, bb, l);
			}
		}
	}
	painter.end();
}

void PCBView::drawOrigin(QPainter *painter)
{
	// circle with 4 lines
	painter->drawEllipse(XPcb::IN2PCB(-0.05),XPcb::IN2PCB(-0.05),XPcb::IN2PCB(0.1), XPcb::IN2PCB(0.1));
	painter->drawLine(XPcb::IN2PCB(0.05), 0, XPcb::IN2PCB(0.25), 0);
	painter->drawLine(0, XPcb::IN2PCB(0.05), 0, XPcb::IN2PCB(0.25));
	painter->drawLine(XPcb::IN2PCB(-0.05), 0, XPcb::IN2PCB(-0.25), 0);
	painter->drawLine(0, XPcb::IN2PCB(-0.05), 0, XPcb::IN2PCB(-0.25));
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

void PCBView::enterEvent(QEvent *event)
{
	setFocus(Qt::MouseFocusReason);
}

void PCBView::leaveEvent(QEvent *event)
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
								.arg(vp.left()).arg(vp.right()).arg(vp.top()).arg(vp.bottom()));
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
								QPoint(XPcb::PCB_BOUND, XPcb::PCB_BOUND))).contains(this->rect())
		&& factor < 1 )
	{
		return;
	}
	mTransform = test;
	recenter(p, true);
}

QColor PCBView::layerColor(XPcb::PCBLAYER l)
{
	QSettings s;
	return s.value(QString("colors/%1").arg(XPcb::layerName(l))).value<QColor>();
}
