class Pad
{
%TypeHeaderCode
#include "Shape.h"
%End
public:
	enum PADSHAPE {
		PAD_NONE = 0,
		PAD_DEFAULT,
		PAD_ROUND,
		PAD_SQUARE,
		PAD_RECT,
		PAD_RRECT,
		PAD_OBROUND,
		PAD_OCTAGON,
	};

	enum PADCONNTYPE {
		CONN_DEFAULT = 0,	///< use global setting
		CONN_NEVER,			///< never connect pad to area
		CONN_THERMAL,		///< connect pad using a thermal structure
		CONN_NOTHERMAL		///< flood pad with copper
	};

	Pad(PADSHAPE shape = PAD_NONE, int width = 0, int length = 0, int radius = 0, PADCONNTYPE connType = CONN_DEFAULT);
	bool operator==(const Pad &p) const;
	bool isNull() const; 
	bool isDefault() const;

	static Pad newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	PADSHAPE shape();
	int width();
	int length();
	int radius();
	PADCONNTYPE connFlag();
	void setConnFlag(PADCONNTYPE flag);

	bool testHit( const QPoint & pt );

	QRect bbox() const;
	void draw(QPainter *painter) const;
};


class Padstack
{
%TypeHeaderCode
#include "Shape.h"
%End
public:
	Padstack();
	bool operator==(const Padstack &p) const;

	static Padstack* newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	QString name() const; 
	void setName(QString name);

	int holeSize() const;
	void setHoleSize(int size);

	Pad& startPad();
	Pad& endPad();
	Pad& innerPad();
	Pad& startMask();
	Pad& endMask();
	Pad& startPaste();
	Pad& endPaste();
	bool isSmt() const;
	QRect bbox() const;
	void draw(QPainter *painter, const Layer& layer) const;

	int getid() const;
};

class Pin : PCBObject
{
%TypeHeaderCode
#include "Shape.h"
%End
public:

	Pin(Footprint* fp); 

	int angle() const;
	QPoint pos() const;
	Padstack* padstack() const;
	QString name() const;

	void setName(QString name);
	void setPos(QPoint pos);
	void setAngle(int angle);
	void setPadstack(Padstack* ps);

	virtual void accept(PCBObjectVisitor *v);
	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	virtual bool testHit(QPoint pt, const Layer& layer) const;

//	static Pin* newFromXML(QXmlStreamReader &reader, const QHash<int, Padstack*> &padstacks, Footprint* fp);
	void toXML(QXmlStreamWriter &writer) const;

	Pad getPadOnLayer(const Layer& layer) const;

	virtual QTransform transform() const;
private:
	Pin(const Pin& other);
};

class Footprint 
{
%TypeHeaderCode
#include "Shape.h"
%End
public:
	Footprint();
	
	~Footprint();
	%MethodCode
		// check if there is an outstanding shared pointer
		if (SharedPtrMgr::hasRef(sipCppV))
		{
			// decrement the reference count and get out
			SharedPtrMgr::decRef(sipCppV);
			return;
		}
		// otherwise let python deal with it
	%End

	enum FP_DRAW_LAYER { LAY_START, LAY_INNER, LAY_END };

	void draw(QPainter *painter, FP_DRAW_LAYER layer) const;
	QRect bbox() const;

	QString name() const;
	void setName(QString name);

	QString author() const;
	void setAuthor(QString author);

	QString source() const;
	void setSource(QString src);

	QString desc() const;
	void setDesc(QString desc);

	QList<Padstack*> padstacks();
	void addPadstack(Padstack* ps);
	void removePadstack(Padstack* ps);

	int numPins() const;
	const Pin* getPin(const QString & pin) const;
	const Pin* getPin(int i);
	const QList<Pin*> pins();
	void addPin(Pin* p);
	void removePin(Pin* p);

	const QList<Text*> texts();
	void addText(Text* t);
	void removeText(Text* t);

	const QList<Line*> getLines();

	void addLine(Line* l);
	void removeLine(Line* l);

	QRect getPinBounds() const;

	const Text& getRefText();
	const Text& getValueText();

	QPoint centroid();
	bool isCustomCentroid();
	XPcb::UNIT units();

//	static Footprint* newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	const QUuid& uuid() const;
private:
	Footprint(const Footprint& other);
};

class FPDatabase
{
%TypeHeaderCode
#include "Shape.h"
%End
public:
	static FPDatabase& instance();

	QList<FPDBFolder*> rootFolders() const;
	const FPDBFile* getByUuid(QUuid uuid) const;

private:
	FPDatabase();

	FPDBFolder* createFolder(QString path, bool fullName = false);
	FPDBFile* createFile(QString path);
};

class FPDBFile
{
%TypeHeaderCode
#include "Shape.h"
%End
public:
	FPDBFile(QString path, QString name, QString author, QString source, QString desc, QUuid uuid);

	QString path() const;
	QString name() const;
	QString author() const;
	QString source() const;
	QString desc() const;
	QUuid uuid() const;

	QSharedPointer<Footprint> loadFootprint() /Factory/;

	void setParent(FPDBFolder* parent);
	FPDBFolder* parent() const;
};

class FPDBFolder
{
%TypeHeaderCode
#include "Shape.h"
%End
public:
	FPDBFolder(QString name, QList<FPDBFolder*> folders, QList<FPDBFile*> files);
	~FPDBFolder();

	void setParent(FPDBFolder* parent);
	FPDBFolder* parent() const;

	QString name() const;
	QList<FPDBFolder*> folders() const;
	QList<FPDBFile*> items() const;
};


