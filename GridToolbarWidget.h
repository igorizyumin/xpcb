#ifndef GRIDTOOLBARWIDGET_H
#define GRIDTOOLBARWIDGET_H

#include "ui_GridToolbarWidget.h"

class GridToolbarWidget : public QWidget, private Ui::GridToolbarWidget
{
    Q_OBJECT

public:
    explicit GridToolbarWidget(QWidget *parent = 0);

signals:
	void viewGridChanged(int grid);
	void placeGridChanged(int grid);
	void routeGridChanged(int grid);
private:
	int parseUnits(QString str);
private slots:
	void onViewGrid(QString str);
	void onPlaceGrid(QString str);
	void onRouteGrid(QString str);
};

#endif // GRIDTOOLBARWIDGET_H
