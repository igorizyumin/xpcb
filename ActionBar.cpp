#include "ActionBar.h"

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
		QAction *a = new QAction(this);
		a->setShortcut(keys[i]);
		a->setVisible(false);
		connect(a, SIGNAL(changed()), this, SLOT(onActionChanged()));
		connect(buttons[i], SIGNAL(clicked()), a, SLOT(trigger()));
		this->addAction(a);
		mActions.append(a);
	}
}

void ActionBar::onActionChanged()
{
	QPushButton* buttons[8] = {this->butF1, this->butF2, this->butF3, this->butF4,
							   this->butF5, this->butF6, this->butF7, this->butF8 };
	for(int i = 0; i < 8; i++)
	{
		if (!mActions[i]->isVisible())
		{
			buttons[i]->setText("");
			buttons[i]->setEnabled(false);
		}
		else
		{
			buttons[i]->setText(mActions[i]->text());
			buttons[i]->setEnabled(mActions[i]->isEnabled());
		}
	}
}

QString ActionBar::wrapText(QString text)
{
	return text.replace(' ', '\n');
}
