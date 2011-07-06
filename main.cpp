/*
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui/QApplication>
#include "mainwindow.h"
#include "ManagePadstacksDialog.h"
#include "Document.h"
#include <QSettings>

inline void setDefaultValue(QSettings &s, QString key, QVariant value)
{
	// if (!s.contains(key)) s.setValue(key, value);
	s.setValue(key, value);
}

void initSettings()
{
	QSettings s;
	setDefaultValue(s, "colors/selection", QColor(255, 255, 255));
	setDefaultValue(s, "colors/background", QColor(0, 0, 0));
	setDefaultValue(s, "colors/visible grid", QColor(255, 255, 255));
	setDefaultValue(s, "colors/drc error", QColor(255, 128, 64));
	setDefaultValue(s, "colors/board outline", QColor(0, 0, 255));
	setDefaultValue(s, "colors/rat line", QColor(255, 0, 255));
	setDefaultValue(s, "colors/top silk", QColor(255, 255, 0));
	setDefaultValue(s, "colors/bottom silk", QColor(255, 192, 192));
	setDefaultValue(s, "colors/top sm cutout", QColor(160,160,160));
	setDefaultValue(s, "colors/bot sm cutout", QColor(95, 95, 95));
	setDefaultValue(s, "colors/drilled hole", QColor(0, 0, 255));
	setDefaultValue(s, "colors/top copper", QColor(0, 210, 0));
	setDefaultValue(s, "colors/bottom copper", QColor(210, 0, 0));
	setDefaultValue(s, "colors/inner 1", QColor(0, 190, 0));
	setDefaultValue(s, "colors/inner 2", QColor(190, 0, 0));
	setDefaultValue(s, "colors/inner 3", QColor(0, 170, 0));
	setDefaultValue(s, "colors/inner 4", QColor(170, 0, 0));
	setDefaultValue(s, "colors/inner 5", QColor(0, 150, 0));
	setDefaultValue(s, "colors/inner 6", QColor(150, 0, 0));
	setDefaultValue(s, "colors/inner 7", QColor(0, 130, 0));
	setDefaultValue(s, "colors/inner 8", QColor(130, 0, 0));
	setDefaultValue(s, "colors/inner 9", QColor(0, 110, 0));
	setDefaultValue(s, "colors/inner 10", QColor(110, 0, 0));
	setDefaultValue(s, "colors/inner 11", QColor(119, 0, 220));
	setDefaultValue(s, "colors/inner 12", QColor(5, 0, 220));
	setDefaultValue(s, "colors/inner 13", QColor(0, 150, 220));
	setDefaultValue(s, "colors/inner 14", QColor(220, 145, 0));
	//      setDefaultValue(s, "colors/inner 15", QColor(220, 83, 0));
	//      setDefaultValue(s, "colors/inner 16", QColor(220, 0, 140));
	setDefaultValue(s, "colors/top paste", QColor(0, 150, 0));
	setDefaultValue(s, "colors/bottom paste", QColor(150, 0, 0));
	setDefaultValue(s, "colors/start pad", QColor(0, 210, 0));
	setDefaultValue(s, "colors/inner pad", QColor(220, 145, 0));
	setDefaultValue(s, "colors/end pad", QColor(210, 0, 0));
	setDefaultValue(s, "colors/centroid", QColor(255, 255, 255));
	setDefaultValue(s, "colors/adhesive", QColor(231, 231, 231));
	setDefaultValue(s, "colors/unknown layer", QColor(255, 0, 0));
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("xpcb");
	a.setOrganizationDomain("xpcb.org");
	a.setOrganizationName("xpcb.org");

	// initialize the app settings with defaults, if they do not exist
	initSettings();

	PCBEditWindow w;
	w.show();

	return a.exec();
}
