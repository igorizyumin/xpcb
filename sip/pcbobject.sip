
class PCBObjState /Abstract/
{
%TypeHeaderCode
#include "PCBObject.h"
%End
public:
	virtual ~PCBObjState();
protected:
	PCBObjState();
};

class PCBObject /Abstract/
{
%TypeHeaderCode
#include "PCBObject.h"
%End
public:
	PCBObject();

	virtual void accept(PCBObjectVisitor *v) = 0;

	virtual void draw(QPainter *painter, const Layer& layer) const = 0;

	virtual QRect bbox() const = 0;

	int getid() const;

	virtual bool testHit(QPoint /* pt */, const Layer& /*l*/) const;

	virtual QTransform transform() const;

	virtual void parentChanged();

//	virtual QSharedPointer<PCBObjState> getState() const = 0;
	
//	virtual bool loadState(QSharedPointer<PCBObjState> &state) = 0;

	static int getNextID();
};

class PCBObjectVisitor /Abstract/
{
%TypeHeaderCode
#include "PCBObject.h"
%End
public:
//	virtual void visit(Area*) = 0;
//	virtual void visit(Line*) = 0;
//	virtual void visit(Net*) = 0;
//	virtual void visit(PartPin*) = 0;
//	virtual void visit(Pin*) = 0;
//	virtual void visit(Part*) = 0;
//	virtual void visit(Footprint*) = 0;
//	virtual void visit(Text*) = 0;
//	virtual void visit(Vertex*) = 0;
//	virtual void visit(Segment*) = 0;
//	virtual void visit(Padstack*) = 0;
};


