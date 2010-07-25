#ifndef GRIDTOOLBARWIDGET_H
#define GRIDTOOLBARWIDGET_H

#include "ui_GridToolbarWidget.h"

class GridToolbarWidget : public QWidget, private Ui::GridToolbarWidget
{
    Q_OBJECT

public:
    explicit GridToolbarWidget(QWidget *parent = 0);
};

#endif // GRIDTOOLBARWIDGET_H
