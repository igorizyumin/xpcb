#include "global.h"
#include "PCBView.h"
#include "PCBDoc.h"
#include <QSize>
#include <QPainter>
#include <QMouseEvent>

PCBView::PCBView(QWidget *parent)
	: QWidget(parent), mDoc(NULL), mWheelAngle(0)
{
	// initialize transform
	// 100 pixels = 1 inch
	mTransform.translate(0, 300);
	mTransform.scale(100.0/IN2PCB(1), -100.0/IN2PCB(1));
	mVisibleGrid = IN2PCB(0.1);
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

void PCBView::setDoc(PCBDoc *doc)
{
	mDoc = doc;
	if (doc)
		connect(doc, SIGNAL(changed()), this, SLOT(docChanged()));
	docChanged();
}

void PCBView::docChanged()
{
	update();
}

void PCBView::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
//	painter.setBackgroundMode(Qt::BGMode::OpaqueMode);
	painter.setBackground(QBrush(Qt::black));
	painter.setClipping(true);
	painter.eraseRect(this->rect());
	if (mDoc)
	{
		QPen pen(Qt::white);
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		painter.setTransform(mTransform);
		painter.setPen(pen);
		drawOrigin(&painter);
		drawGrid(&painter);
		painter.setRenderHint(QPainter::Antialiasing);
		for(int l = 0; l < MAX_LAYERS; l++)
			mDoc->draw(&painter, mTransform.inverted().mapRect(e->rect()), (PCBLAYER)l);
	}
	painter.end();
}

void PCBView::drawOrigin(QPainter *painter)
{
	// circle with 4 lines
	painter->drawEllipse(IN2PCB(-0.05),IN2PCB(-0.05),IN2PCB(0.1), IN2PCB(0.1));
	painter->drawLine(IN2PCB(0.05), 0, IN2PCB(0.25), 0);
	painter->drawLine(0, IN2PCB(0.05), 0, IN2PCB(0.25));
	painter->drawLine(IN2PCB(-0.05), 0, IN2PCB(-0.25), 0);
	painter->drawLine(0, IN2PCB(-0.05), 0, IN2PCB(-0.25));
}

void PCBView::drawGrid(QPainter *painter)
{
	QRect viewport = mTransform.inverted().mapRect(this->rect());
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
	QPoint ctr = mTransform.inverted().map(this->rect().center());
	if (!world)
		pt = mTransform.inverted().map(pt);
	ctr -= pt;
	mTransform.translate(ctr.x(), ctr.y());
	// move cursor to middle of the window
	QCursor::setPos(mapToGlobal(rect().center()));
	// force repaint
	update();
}

/// Pos is in screen coords
void PCBView::zoom(double factor, QPoint pos)
{
	recenter(pos);
	QPoint p = mTransform.inverted().map(rect().center());
	mTransform.scale(factor, factor);
	recenter(p, true);
}
