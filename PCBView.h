#ifndef QPCBVIEW_H
#define QPCBVIEW_H

#include <QWidget>
#include <QTransform>

class PCBDoc;

class PCBView : public QWidget
{
	Q_OBJECT

public:
        PCBView(QWidget *parent);
        ~PCBView();


	virtual QSize sizeHint() const;
	void setDoc(PCBDoc* doc);

signals:
	void mouseMoved(QPoint pt);

private slots:
	void docChanged();

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);
	virtual void wheelEvent(QWheelEvent *);

private:
	void drawOrigin(QPainter *painter);
	void drawGrid(QPainter *painter);
	void recenter(QPoint pt, bool world=false);
	void zoom(double factor, QPoint pos);

	/// Document
	PCBDoc* mDoc;

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
