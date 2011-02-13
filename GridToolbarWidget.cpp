#include "GridToolbarWidget.h"
#include "global.h"

GridToolbarWidget::GridToolbarWidget(QWidget *parent) :
	QWidget(parent)
{
    setupUi(this);
	connect(this->placeGridBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onPlaceGrid(QString)));
	connect(this->routGridBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onRouteGrid(QString)));
	connect(this->visGridBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onViewGrid(QString)));
}

int GridToolbarWidget::parseUnits(QString str)
{
	bool in_mm = false;
	if (str.contains("mm")) in_mm = true;
	double val = str.remove(" mm").toDouble();
	if (in_mm) return XPcb::MM2PCB(val);
	else return XPcb::IN2PCB(val/1000);
}

void GridToolbarWidget::onViewGrid(QString str)
{
	emit viewGridChanged(parseUnits(str));
}

void GridToolbarWidget::onPlaceGrid(QString str)
{
	emit placeGridChanged(parseUnits(str));
}

void GridToolbarWidget::onRouteGrid(QString str)
{
	emit routeGridChanged(parseUnits(str));
}
