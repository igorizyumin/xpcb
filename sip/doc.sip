class Document : QObject /Abstract/
{
%TypeHeaderCode
#include "PCBDoc.h"
%End
%ConvertToSubClassCode
	if (dynamic_cast<PCBDoc*>(sipCpp))
		sipType = sipType_PCBDoc;
	else if (dynamic_cast<FPDoc*>(sipCpp))
		sipType = sipType_FPDoc;
	else
		sipType = NULL;
%End
public:
	enum LayerOrder
	{
		ListOrder,
		DrawPriorityOrder
	};
	Document();
	virtual ~Document();

	virtual bool saveToFile(const QString & file);
	virtual bool saveToXml(QXmlStreamWriter &writer) = 0;
	
	virtual bool loadFromFile(const QString & file);
	virtual bool loadFromFile(QFile & file) = 0;
	virtual bool loadFromXml(QXmlStreamReader &reader) = 0;

	virtual bool isModified();

	virtual void doCommand(QUndoCommand *cmd /Transfer/);

	virtual QList<Layer> layerList(LayerOrder order = ListOrder) = 0;

	virtual QString name() const;
	virtual XPcb::UNIT units() const;

	virtual QList<QSharedPointer<PCBObject> > findObjs(QPoint &pt) = 0;
	virtual QList<QSharedPointer<PCBObject> > findObjs(QRect &rect) = 0;

	virtual void addText(QSharedPointer<Text> t) = 0;
	virtual void removeText(QSharedPointer<Text> t) = 0;

signals:
	void changed();
	void cleanChanged(bool clean);
	void canUndoChanged(bool e);
	void canRedoChanged(bool e);

public slots:
	virtual void undo();
	virtual void redo();
};

class FPDoc : Document
{
public:
	FPDoc();
	virtual ~FPDoc();

	virtual bool loadFromFile(const QString & file);
	virtual bool saveToFile(const QString & file);
	virtual bool saveToXml(QXmlStreamWriter &writer);
	virtual bool loadFromFile(QFile & file);
	virtual bool loadFromXml(QXmlStreamReader &reader);

	virtual QList<Layer> layerList(LayerOrder order = ListOrder);

	virtual QList<QSharedPointer<PCBObject> > findObjs(QPoint &pt);
	virtual QList<QSharedPointer<PCBObject> > findObjs(QRect &rect);

	virtual void addText(QSharedPointer<Text> t);
	virtual void removeText(QSharedPointer<Text> t);

//	virtual QList<Padstack*> padstacks();
//	virtual void addPadstack(Padstack* ps);
//	virtual void removePadstack(Padstack* ps);

	void addPin(QSharedPointer<Pin> p);
	void removePin(QSharedPointer<Pin> p);

	void addLine(QSharedPointer<Line> l);
	void removeLine(QSharedPointer<Line> l);

	QSharedPointer<Footprint> footprint();
};

class PCBDoc : Document
{
public:
    PCBDoc();
	virtual ~PCBDoc();

	// overrides
	virtual bool loadFromFile(const QString & file);
	virtual bool saveToFile(const QString & file);
	virtual bool saveToXml(QXmlStreamWriter &writer);
	virtual bool loadFromFile(QFile & file);
	virtual bool loadFromXml(QXmlStreamReader &reader);
	virtual QList<Layer> layerList(LayerOrder order = ListOrder);

	virtual QList<QSharedPointer<PCBObject> > findObjs(QPoint &pt);
	virtual QList<QSharedPointer<PCBObject> > findObjs(QRect &rect);

	TraceList& traceList() const; 
	QSharedPointer<Part> part(const QString & refdes);
	QSharedPointer<Footprint> getFootprint(QUuid uuid);
	QList<QSharedPointer<Footprint> > footprints();

	QSharedPointer<Net> net(const QString &name) const;

	void addText(QSharedPointer<Text> t);
	void removeText(QSharedPointer<Text> t);

	void addPart(QSharedPointer<Part> p);
	void removePart(QSharedPointer<Part> p);

	int numLayers() const;

//	virtual QList<Padstack*> padstacks() { return mPadstacks; }
//	virtual void addPadstack(Padstack* ps);
//	virtual void removePadstack(Padstack* ps);
};

