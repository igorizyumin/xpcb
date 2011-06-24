
class Net : PCBObject
{
%TypeHeaderCode
#include "Net.h"
%End
public:
	Net( PCBDoc *doc, const QString &name);
	~Net();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v); 
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	QString name() const; 
	void addPin( PartPin * pin);
	void removePin( PartPin * pin);
	QSet<PartPin*> getPins() const; 
	Padstack* getViaPs() const;
	bool visible() const;
	void setVisible(bool v);

//	static Net* newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
//						   const QHash<int, Padstack*> &padstacks);
	void toXML(QXmlStreamWriter &writer) const;
};


class Netlist
{
%TypeHeaderCode
#include "Net.h"
%End

public:
	void addNet(QString name, QList<QPair<QString, QString> > net); 
	void removeNet(QString name); 
	QList<QPair<QString, QString> > net(QString &name) const;

	/// Returns a list of part reference designators.
	QList<QString> parts() const;
	/// Returns the footprint name for a part.
	QString partFpName(QString refdes) const;

	void addPart(QString refdes, QString fp);
};

class NetlistLoader
{
%TypeHeaderCode
#include "Net.h"
%End
public:
	static Netlist* loadFile(QString path) /Factory/;
	static void registerLoader(NetlistLoader* loader /Transfer/);
protected:
	virtual Netlist* loadFromFile(QFile &file) = 0 /Factory/;
};

