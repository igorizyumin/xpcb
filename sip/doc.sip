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

	virtual QList<PCBObject*> findObjs(QPoint &pt) = 0;
	virtual QList<PCBObject*> findObjs(QRect &rect) = 0;

	virtual void addText(Text* t /Transfer/) = 0;
	virtual void removeText(Text* t /TransferBack/) = 0;

//	virtual QList<Padstack*> padstacks() = 0;

//	virtual void addPadstack(Padstack* ps) = 0;

//	virtual void removePadstack(Padstack* ps) = 0;
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

	virtual QList<PCBObject*> findObjs(QPoint &pt);
	virtual QList<PCBObject*> findObjs(QRect &rect);

	virtual void addText(Text* t /Transfer/);
	virtual void removeText(Text* t /TransferBack/);

//	virtual QList<Padstack*> padstacks();
//	virtual void addPadstack(Padstack* ps);
//	virtual void removePadstack(Padstack* ps);

	void addPin(Pin* p /Transfer/);
	void removePin(Pin* p /TransferBack/);

	void addLine(Line* l /Transfer/);
	void removeLine(Line* l /TransferBack/);

	Footprint* footprint();
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
	virtual QList<PCBObject*> findObjs(QPoint &pt);
	virtual QList<PCBObject*> findObjs(QRect &rect);

	TraceList& traceList() const; 
	Part* getPart(const QString & refdes);
	QSharedPointer<Footprint> getFootprint(QUuid uuid) /Factory/;
	QList<QSharedPointer<Footprint> > footprints() /Factory/;
	Net* getNet(const QString &name) const;

	void addText(Text* t /Transfer/);
	void removeText(Text* t /TransferBack/);

	void addPart(Part* p /Transfer/);
	void removePart(Part* p /TransferBack/);

	int numLayers() const;

//	virtual QList<Padstack*> padstacks() { return mPadstacks; }
//	virtual void addPadstack(Padstack* ps);
//	virtual void removePadstack(Padstack* ps);
};

