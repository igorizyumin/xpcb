#ifndef QPCBVIEW_H
#define QPCBVIEW_H

#include <QWidget>
#include <QTransform>
#include "global.h"

class PCBDoc;
class PCBObject;
class Controller;

class PCBView : public QWidget
{
	Q_OBJECT

public:
        PCBView(QWidget *parent);
        ~PCBView();


	virtual QSize sizeHint() const;

	/// This gets called by the controller when the view is registered.
	void setCtrl(Controller* ctrl);

	const QTransform& transform() const { return mTransform; }

signals:
	void mouseMoved(QPoint pt);

public slots:
	void visGridChanged(int grid);
	//	void layerVisChanged();

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);
	virtual void wheelEvent(QWheelEvent *);

private:
	void drawOrigin(QPainter *painter);
	void drawGrid(QPainter *painter);
	void recenter(QPoint pt, bool world=false);
	void zoom(double factor, QPoint pos);

	QColor layerColor(XPcb::PCBLAYER l);

	/// Controller
	Controller* mCtrl;

	/// Transform from world coordinates to window coordinates
	QTransform mTransform;
	/// Visible grid size in PCB units
	int mVisibleGrid;
	/// Mouse position (world coordinates)
	QPoint mMousePos;
	/// Accumulated mouse wheel angle
	int mWheelAngle;
};

#endif // QPCBVIEW_H
