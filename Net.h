#ifndef NET_H
#define NET_H

#include <QString>
#include <QList>
#include <QSet>
#include "global.h"
#include "Part.h"
#include "Trace.h"

// Basic types



class Net : public PCBObject
{
public:
	Net( QString name, int def_width, int def_via_w, int def_via_hole_w );
	~Net();

	QString getName() {return name;}
//	void moveOrigin(QPoint delta);
	void AddPin( PartPin * pin, bool set_areas=true );
	PartPin* TestHitOnPin( QPoint pt, PCBLAYER layer);
	QSet<PartPin*> getPins() { return mPins; }

private:
	QString mName;		// net name
	QSet<PartPin*> mPins;	// pointers to part pins that are in this net
	int mDefW;			// default trace width
	int mDefViaW;		// default via width
	int mDefViaHole;	// default via hole width
};

#endif // NET_H
