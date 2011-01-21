#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <QWidget>
#include "ui_ActionBar.h"

class ActionBar : public QWidget, private Ui::ActionBar
{
    Q_OBJECT

public:
    explicit ActionBar(QWidget *parent = 0);

private:

};

#endif // ACTIONBAR_H
