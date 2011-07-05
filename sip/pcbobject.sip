
class PCBObjState
{
%TypeHeaderCode
#include "PCBObject.h"
%End
	PCBObjState();
};

class PCBObject /Abstract/
{
%TypeHeaderCode
#include "PCBObject.h"
%End
%ConvertToSubClassCode
	if (dynamic_cast<Line*>(sipCpp))
		sipType = sipType_Line;
	else if (dynamic_cast<Net*>(sipCpp))
		sipType = sipType_Net;
	else if (dynamic_cast<PartPin*>(sipCpp))
		sipType = sipType_PartPin;
	else if (dynamic_cast<Pin*>(sipCpp))
		sipType = sipType_Pin;
	else if (dynamic_cast<Part*>(sipCpp))
		sipType = sipType_Part;
	else if (dynamic_cast<Text*>(sipCpp))
		sipType = sipType_Text;
	else if (dynamic_cast<Vertex*>(sipCpp))
		sipType = sipType_Vertex;
	else if (dynamic_cast<Segment*>(sipCpp))
		sipType = sipType_Segment;
	else
		sipType = NULL;
%End
public:
	PCBObject();
	~PCBObject();
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


	virtual void draw(QPainter *painter, const Layer& layer) const = 0;
	virtual QRect bbox() const = 0;
	int getid() const;
	virtual bool testHit(QPoint /* pt */, const Layer& /*l*/) const;
	virtual QTransform transform() const;

	virtual void parentChanged();

	virtual PCBObjState getState() const = 0;
	virtual bool loadState(PCBObjState &state) = 0;

	static int getNextID();
};

class AbstractEditorFactory
{
%TypeHeaderCode
#include "Editor.h"
%End
public:
	~AbstractEditorFactory();
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

	virtual QSharedPointer<AbstractEditor> makeEditor(Controller* ctrl, QSharedPointer<PCBObject> obj) = 0 /Factory/;
};

class EditorFactory /Abstract/
{
%TypeHeaderCode
#include "Editor.h"
%End
public:
	enum ObjType {ObjArea, ObjLine, ObjNet, ObjPartPin, ObjPin, ObjPart, ObjFootprint, ObjText, ObjVertex, ObjSegment, ObjPadstack};
	static EditorFactory& instance();

	QSharedPointer<AbstractEditor> newEditor(QSharedPointer<PCBObject> obj, Controller *ctrl);
	QSharedPointer<AbstractEditor> newTextEditor(Controller *ctrl);
//	QSharedPointer<AbstractEditor> newPinEditor(FPController* ctrl);
//	QSharedPointer<AbstractEditor> newPartEditor(PCBController* ctrl);

	static void registerFactory(ObjType type, QSharedPointer<AbstractEditorFactory> factory /Transfer/);

private:
	EditorFactory();
	EditorFactory(EditorFactory &other);
};

class PCBObjEditCmd : QUndoCommand
{
%TypeHeaderCode
#include "PCBObject.h"
%End
public:
	PCBObjEditCmd(QUndoCommand* parent, QSharedPointer<PCBObject> obj,
			  PCBObjState prevState);

	virtual void undo();
	virtual void redo();
};

class Text : PCBObject 
{
%TypeHeaderCode
#include "Text.h"
%End
public:
	Text();
	Text(const QPoint &pos, int angle,
		bool mirror, bool negative, const Layer& layer, int font_size,
		int stroke_width, const QString &text );
	~Text();
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

	void setParent(PCBObject *parent);

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual bool testHit(QPoint pt, const Layer& l) const;
	virtual PCBObjState getState() const; 
	virtual bool loadState(PCBObjState &state);

	static QSharedPointer<Text> newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	QPoint pos() const;
	void setPos(const QPoint &newpos);

	const QString & text() const;
	void setText(const QString &text);

	int angle() const;
	void setAngle(int angle);

	int fontSize() const;
	void setFontSize(int size);

	int strokeWidth() const;
	void setStrokeWidth(int w);

	bool isMirrored() const;
	void setMirrored(bool b);

	bool isNegative() const;
	void setNegative(bool b);

	const Layer& layer() const;
	void setLayer(const Layer& l);


	virtual void parentChanged();

protected:
	void changed();
	void rebuild() const;

};

class Line : PCBObject
{
%TypeHeaderCode
#include "Line.h"
%End
public:
	enum LineType { LINE, ARC_CW, ARC_CCW };

    Line();
	~Line();
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

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);
	virtual bool testHit(QPoint p, const Layer &l) const;

	QPoint start() const;
	void setStart(QPoint p);
	QPoint end() const;
	void setEnd(QPoint p);
	int width() const;
	void setWidth(int width);
	const Layer& layer() const;
	void setLayer(const Layer& l);
	LineType type() const;
	void setType(LineType t);

	static QSharedPointer<Line> newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	static void drawArc(QPainter* painter, QPoint start, QPoint end, LineType type);
};

