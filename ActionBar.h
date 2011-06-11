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

        void setActions(QList<const CtrlAction*> actions);
        void setActions(const CtrlAction* action);
	void clearActions();

private:
	QString wrapText(QString text);
};

#endif // ACTIONBAR_H
