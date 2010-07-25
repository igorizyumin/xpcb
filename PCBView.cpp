#include "PCBView.h"
#include <QSize>
#include <QPainter>

PCBView::PCBView(QWidget *parent)
	: QWidget(parent)
{
}

PCBView::~PCBView()
{

}

QSize PCBView::sizeHint() const
{
	return QSize(800,600);
}

void PCBView::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
//	painter.setRenderHint(QPainter::Antialiasing);
//	painter.setBackgroundMode(Qt::BGMode::OpaqueMode);
//	painter.setClipping(true);
//	m_view->m_Doc->m_dlist->SetDCToWorldCoords(&painter, m_view->m_org_x, m_view->m_org_y);
//	m_view->m_Doc->m_dlist->Draw(&painter);
	painter.end();
}

