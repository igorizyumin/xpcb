/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
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

#ifndef PLUGIN_H
#define PLUGIN_H

class FPEditWindow;
class PCBEditWindow;

class Plugin
{
public:
    Plugin();

	virtual void installWidgets(FPEditWindow& window) = 0;
	virtual void installWidgets(PCBEditWindow& window) = 0;


};

#endif // PLUGIN_H
