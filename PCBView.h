#ifndef QPCBVIEW_H
#define QPCBVIEW_H

#include <QWidget>

class PCBView : public QWidget
{
	Q_OBJECT

public:
        PCBView(QWidget *parent);
        ~PCBView();

	virtual QSize sizeHint() const;

protected:
	virtual void paintEvent(QPaintEvent *e);	
};

#endif // QPCBVIEW_H
