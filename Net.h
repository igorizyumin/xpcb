#ifndef NET_H
#define NET_H

#include <QString>
#include <QList>
#include <QSet>
#include "global.h"
#include "Part.h"
#include "Trace.h"

// Basic types



class Net
{
public:
	Net( QString name, int def_width, int def_via_w, int def_via_hole_w );
	~Net();

	QString getName() {return name;}
//	void moveOrigin(QPoint delta);
	void AddPin( PartPin * pin, bool set_areas=true );
//	PartPin* TestHitOnPin( QPoint pt, PCBLAYER layer);

private:
	QString name;		// net name
//	QList<Connection> connect; // array of connections (size = max_pins-1)
	QSet<PartPin*> pin;	// pointers to part pins that are in this net
//	QList<Area> area;	// array of copper areas
	int def_w;			// default trace width
	int def_via_w;		// default via width
	int def_via_hole_w;	// default via hole width
	bool visible;		// false to hide ratlines and make unselectable
};

#endif // NET_H
