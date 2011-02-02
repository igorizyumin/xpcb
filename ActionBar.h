#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <QWidget>
#include <QSignalMapper>

#include "Controller.h"
#include "ui_ActionBar.h"

class ActionBar : public QWidget, private Ui::ActionBar
{
    Q_OBJECT

public:
    explicit ActionBar(QWidget *parent = 0);

	void setActions(QList<CtrlAction> actions);
	void setActions(CtrlAction action);

signals:
	void triggered(int key);

private:
//	QList<QAction*> mActions;
	QString wrapText(QString text);
	QSignalMapper mMapper;

};

#endif // ACTIONBAR_H
