
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
public:
	PCBObject();

	virtual void accept(PCBObjectVisitor *v) = 0;

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
//	virtual void visit(Text*) = 0;
//	virtual void visit(Vertex*) = 0;
//	virtual void visit(Segment*) = 0;
};

class AbstractEditorFactory
{
%TypeHeaderCode
#include "Editor.h"
%End
public:
	virtual AbstractEditor* makeEditor(Controller* ctrl, PCBObject *obj) = 0 /Factory/;
};

class EditorFactory : PCBObjectVisitor /Abstract/
{
%TypeHeaderCode
#include "Editor.h"
%End
public:
	enum ObjType {ObjArea, ObjLine, ObjNet, ObjPartPin, ObjPin, ObjPart, ObjFootprint, ObjText, ObjVertex, ObjSegment, ObjPadstack};
	static EditorFactory& instance();

	AbstractEditor* newEditor(PCBObject* obj, Controller *ctrl) /TransferBack/;
//	AbstractEditor* newTextEditor(Controller *ctrl);
//	AbstractEditor* newPinEditor(FPController* ctrl);

	static void registerFactory(ObjType type, AbstractEditorFactory* factory /Transfer/);

//	virtual void visit(Area* a);
//	virtual void visit(Line* a);
//	virtual void visit(Net* a);
//	virtual void visit(PartPin* a);
//	virtual void visit(Pin* a);
//	virtual void visit(Part* a);
//	virtual void visit(Text* a);
//	virtual void visit(Vertex* a);
//	virtual void visit(Segment* a);

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
	PCBObjEditCmd(QUndoCommand* parent /TransferThis/, PCBObject *obj /Transfer/,
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

	void setParent(PCBObject *parent);

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v);
	virtual bool testHit(QPoint pt, const Layer& l) const;
	virtual PCBObjState getState() const; 
	virtual bool loadState(PCBObjState &state);

	static Text* newFromXML(QXmlStreamReader &reader);
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

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v);
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

	static Line* newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	static void drawArc(QPainter* painter, QPoint start, QPoint end, LineType type);
};

