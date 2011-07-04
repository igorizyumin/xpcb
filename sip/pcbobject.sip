
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
	virtual QSharedPointer<AbstractEditor> makeEditor(Controller* ctrl, QSharedPointer<PCBObject> obj) = 0;
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

	static void registerFactory(ObjType type, QSharedPointer<AbstractEditorFactory> factory);

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

