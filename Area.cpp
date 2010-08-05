#include "Area.h"
#include "Net.h"
#include "Part.h"
#include "global.h"
#include "PCBDoc.h"


Area::Area()
{
}


void Area::findConnections()
{
	if (!this->mNet)
		return;

	this->mConnPins.clear();

	// test all pins in net for being inside copper area
	PCBLAYER layer = GetLayer();	// layer of copper area
	QSet<PartPin*> pins = mNet->getPins();
	foreach(PartPin* pin, pins)
	{
		// see if pin is on the right layer
		PCBLAYER pin_layer = pin->getLayer();
		if( pin_layer != LAY_PAD_THRU
			&& pin_layer != layer )
			continue;	// SMT pad not on our layer

		// check if pin is allowed to connect to areas
		Part* part = pin->getPart();
		ASSERT(part); // pin not attached to part? what the hell

		// get the pad for the appropriate layer
		Pad pad;
		if (!pin->getPadOnLayer(layer, pad))
			continue; // no pad on this layer

		// see if pad allowed to connect
		Footprint* fp = part->getFootprint();
		if (!fp)
			continue; // error?
		Padstack ps = fp->getPadstack(pin->getName());
		ASSERT(ps);

		if( pad.connect_flag == PAD_CONNECT_NEVER )
			continue;	// pad never allowed to connect
		if( pad.connect_flag == PAD_CONNECT_DEFAULT && !ps.hole_size && !mConnectSMT )
			continue;	// SMT pad, not allowed to connect to this area
		if( pin_layer != LAY_PAD_THRU && pad.shape == PAD_NONE )
			continue;	// no SMT pad defined (this should not happen)

		// see if pad is inside copper area
		QPoint p = pin->getPos();
		if( TestPointInside( p.x(), p.y() ) )
		{
			// pin is inside copper area
			this->mConnPins.append(pin);
		}
	}

	// find all vertices within copper area
	TraceList &tl = this->mDoc->traceList();
	mConnVtx = *tl.getVerticesInPoly(this);
}
