#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <QWidget>
#include <QPushButton>
#include "ui_ActionBar.h"

class ActionBar : public QWidget, private Ui::ActionBar
{
    Q_OBJECT

public:
    explicit ActionBar(QWidget *parent = 0);
	QList<QAction*> getActions() { return mActions; }

private slots:
	void onActionChanged();

private:
	QList<QAction*> mActions;
	QString wrapText(QString text);

};

#endif // ACTIONBAR_H
