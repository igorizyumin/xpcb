#include "ActionBar.h"
#include <QShortcut>

ActionBar::ActionBar(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
	QPushButton* buttons[8] = {this->butF1, this->butF2, this->butF3, this->butF4,
							   this->butF5, this->butF6, this->butF7, this->butF8 };
	Qt::Key keys[8] = {Qt::Key_F1, Qt::Key_F2, Qt::Key_F3, Qt::Key_F4,
					Qt::Key_F5, Qt::Key_F6, Qt::Key_F7, Qt::Key_F8};
	for(int i = 0; i < 8; i++)
	{
		QShortcut *s = new QShortcut(keys[i], this);
		connect(s, SIGNAL(activated()), buttons[i], SLOT(animateClick()));
		connect(buttons[i], SIGNAL(clicked()), &mMapper, SLOT(map()));
		mMapper.setMapping(buttons[i], i);
	}
	connect(&mMapper, SIGNAL(mapped(int)), this, SIGNAL(triggered(int)));
}

void ActionBar::setActions(QList<CtrlAction> actions)
{
	QPushButton* buttons[8] = {this->butF1, this->butF2, this->butF3, this->butF4,
							   this->butF5, this->butF6, this->butF7, this->butF8 };
	clearActions();
	foreach(const CtrlAction& a, actions)
	{
		buttons[a.key()]->setText(wrapText(a.text()));
		buttons[a.key()]->setEnabled(true);
	}
}

void ActionBar::setActions(CtrlAction action)
{
	QList<CtrlAction> l;
	l.append(action);
	setActions(l);
}

void ActionBar::clearActions()
{
	QPushButton* buttons[8] = {this->butF1, this->butF2, this->butF3, this->butF4,
							   this->butF5, this->butF6, this->butF7, this->butF8 };
	for(int i = 0; i < 8; i++)
	{
		buttons[i]->setText("");
		buttons[i]->setEnabled(false);
	}
}

QString ActionBar::wrapText(QString text)
{
	return text.replace(' ', '\n');
}
