#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMouseEvent>
#include "Text.h"
#include "Log.h"
#include "smfontutil.h"
#include "Controller.h"
#include "PCBView.h"
#include "PCBDoc.h"
#include "EditTextDialog.h"

Text::Text()
	: PCBObject(), mAngle(0), mIsMirrored(false), mIsNegative(false),
	mFontSize(XPcb::MIL2PCB(100)), mStrokeWidth(XPcb::MIL2PCB(10)), mParent(NULL), mIsDirty(true)
{
}

Text::Text( const QPoint &pos, int angle, bool mirror,
			bool negative, const Layer& layer, int font_size, int stroke_width,
			const QString &str ) :
PCBObject(), mPos(pos), mLayer(layer), mAngle(angle), mIsMirrored(mirror), mIsNegative(negative),
mFontSize(font_size), mStrokeWidth(stroke_width), mText(str), mParent(NULL), mIsDirty(true)
{

}

QPoint Text::pos() const
{
	if (mParent)
		return mParent->transform().map(mPos);
	else return mPos;
}

void Text::setPos(const QPoint &newpos)
{
	if (mParent)
		mPos = mParent->transform().inverted().map(newpos);
	else mPos = newpos;
	changed();
}

void Text::rebuild() const
{
	SMFontUtil::instance().GetStrokes(text(), fontSize(), strokeWidth(),
									  mStrokes, mStrokeBBox);
	double yoffset = 9.0*mFontSize/22.0;
	if (mParent)
		mTransform = mParent->transform();
	else
		mTransform.reset();
	mTransform.translate(mPos.x(), mPos.y());
	if (isMirrored())
		mTransform.scale(-1, 1);
	mTransform.rotate(mAngle);
	mTransform.translate(0, yoffset);
//	if (mParent)
//		mTransform *= mParent->transform();
}

QRect Text::bbox() const
{
	if (mIsDirty)
	{
		rebuild();
		mIsDirty = false;
	}
	return mTransform.mapRect(this->mStrokeBBox);
}

void Text::draw(QPainter *painter, const Layer& layer) const
{
	if (layer != mLayer && layer != Layer::LAY_SELECTION)
		return;

	// reload strokes if text has changed
	if (mIsDirty)
	{
		rebuild();
		mIsDirty = false;
	}

	// save the painter state
	painter->save();

	// set up the transformation
	painter->setTransform(mTransform, true);

	// set up pen/brush
	QPen pen = painter->pen();
	pen.setWidth(mStrokeWidth);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);

	// now draw strokes
	foreach(const QPainterPath& path, this->mStrokes)
	{
		painter->drawPath(path);
	}

	// restore painter state
	painter->restore();
}

Text* Text::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "text");

	QXmlStreamAttributes attr = reader.attributes();
	Text *t = new Text();
	t->mLayer = Layer(attr.value("layer").toString().toInt());
	t->mPos = QPoint(
			attr.value("x").toString().toInt(),
			attr.value("y").toString().toInt());
	t->mAngle = attr.value("rot").toString().toInt();
	t->mStrokeWidth = attr.value("lineWidth").toString().toInt();
	t->mFontSize = attr.value("textSize").toString().toInt();
	t->mText = reader.readElementText();
	t->mIsDirty = true;

	return t;
}

void Text::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("text");
	writer.writeAttribute("layer", QString::number(mLayer.toInt()));
	writer.writeAttribute("x", QString::number(mPos.x()));
	writer.writeAttribute("y", QString::number(mPos.y()));
	writer.writeAttribute("rot", QString::number(mAngle));
	writer.writeAttribute("lineWidth", QString::number(mStrokeWidth));
	writer.writeAttribute("textSize", QString::number(mFontSize));
	writer.writeCharacters(mText);
	writer.writeEndElement();
}

bool Text::loadState(QSharedPointer<PCBObjState> &state)
{
	// convert to text state
	QSharedPointer<TextState> s = state.dynamicCast<TextState>();
	if (s.isNull())
		return false;
	// restore state
	mPos = s->pos;
	mLayer = s->layer;
	mAngle = s->angle;
	mIsMirrored = s->ismirrored;
	mIsNegative = s->isnegative;
	mFontSize = s->fontsize;
	mStrokeWidth = s->width;
	mText = s->text;
	changed();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// EDITOR

TextEditor::TextEditor(Controller *ctrl, Text *text)
		: AbstractEditor(ctrl), mState(SELECTED), mText(text), mDialog(NULL), mAngleDelta(0),
		mRotateAction(2, "Rotate Text"),
		mEditAction(0, "Edit Text"),
		mMoveAction(3, "Move Text"),
		mDeleteAction(7, "Delete Text")
{
	connect(&mRotateAction, SIGNAL(execFired()), SLOT(actionRotate()));
	connect(&mEditAction, SIGNAL(execFired()), SLOT(actionEdit()));
	connect(&mMoveAction, SIGNAL(execFired()), SLOT(actionMove()));
	connect(&mDeleteAction, SIGNAL(execFired()), SLOT(actionDelete()));
}

void TextEditor::init()
{
	emit actionsChanged();
	if (!mText)
		newText();
	else
		ctrl()->hideObj(mText);
}

TextEditor::~TextEditor()
{
	if (mDialog)
		delete mDialog;
}

QList<const CtrlAction*> TextEditor::actions() const
{
		QList<const CtrlAction*> out;

	switch(mState)
	{
	case MOVE:
	case ADD_MOVE:
	case EDIT_MOVE:
				out.append(&mRotateAction);
		break;
	case SELECTED:
				out.append(&mEditAction);
				out.append(&mMoveAction);
				out.append(&mDeleteAction);
		break;
	}
	return out;
}

void TextEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		mPos = ctrl()->snapToPlaceGrid(ctrl()->view()->transform().inverted().map(event->pos()));
		emit overlayChanged();
	}

	// we don't want to eat mouse events
	event->ignore();
}

void TextEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState != SELECTED)
		event->accept();
	else
		event->ignore();
}

void TextEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mState == MOVE)
	{
		event->accept();
		mState = SELECTED;
		TextMoveCmd* cmd = new TextMoveCmd(NULL, mText, mPos, mAngleDelta);
		ctrl()->doc()->doCommand(cmd);
		emit actionsChanged();
		emit overlayChanged();
	}
	else if (mState == EDIT_MOVE)
	{
		event->accept();
		mState = SELECTED;
		finishEdit();
		emit actionsChanged();
	}
	else if (mState == ADD_MOVE)
	{
		event->accept();
		mState = SELECTED;
		finishNew();
		emit actionsChanged();
	}
	else
		event->ignore();
}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState != SELECTED)
	{
		if (mState == ADD_MOVE)
		{
			delete mText;
			mText = NULL;
			emit editorFinished();
		}
		else
		{
			mState = SELECTED;
			emit actionsChanged();
			emit overlayChanged();
		}
		event->accept();
	}
	else
		event->ignore();
}

void TextEditor::actionMove()
{

	mState = MOVE;
	startMove();
	QCursor::setPos(ctrl()->view()->mapToGlobal(ctrl()->view()->transform().map(mPos)));
	emit actionsChanged();
	emit overlayChanged();
}

void TextEditor::startMove()
{
	mPos = mText->pos();
	mAngleDelta = 0;
	mBox = mText->bbox().translated(-mPos);
}

void TextEditor::actionDelete()
{
	TextDeleteCmd* cmd = new TextDeleteCmd(NULL, mText, ctrl()->doc());
	ctrl()->doc()->doCommand(cmd);
	emit editorFinished();
}

void TextEditor::actionRotate()
{
	mAngleDelta += 90;
	mAngleDelta %= 360;
	emit overlayChanged();
}

void TextEditor::newText()
{
	if (!mDialog)
		mDialog = new EditTextDialog(ctrl()->view());
	mDialog->init();
	if (mDialog->exec() == QDialog::Rejected || mDialog->text().isEmpty())
	{
		emit editorFinished();
		return;
	}
	mText = new Text(mDialog->pos(), mDialog->angle(), mDialog->isMirrored(), mDialog->isNegative(),
					 mDialog->layer(), mDialog->textHeight(), mDialog->textWidth(), mDialog->text());
	if (!mDialog->isWidthSet())
		mText->setStrokeWidth(mText->fontSize()*0.1);
	if (!mDialog->isPosSet())
	{
		mState = ADD_MOVE;
		startMove();
		emit actionsChanged();
	}
	else
	{
		mPos = mDialog->pos();
		mAngleDelta = 0;
		finishNew();
	}
}

void TextEditor::actionEdit()
{
	if (!mDialog)
		mDialog = new EditTextDialog(ctrl()->view());
	mDialog->init(mText);
	if (mDialog->exec() == QDialog::Accepted)
	{
		if (mDialog->isPosSet())
		{
			mPos = mDialog->pos();
			mAngleDelta = mDialog->angle() - mText->angle();
			while(mAngleDelta < 0) mAngleDelta += 360;
			finishEdit();
		}
		else // drag to position
		{
			mState = EDIT_MOVE;
			startMove();
			emit actionsChanged();
			emit overlayChanged();
		}
	}
}

void TextEditor::finishNew()
{
	mText->setPos(mPos);
	mText->setAngle(mText->angle() + mAngleDelta);
	TextNewCmd *cmd = new TextNewCmd(NULL, mText, ctrl()->doc());
	ctrl()->doc()->doCommand(cmd);
	ctrl()->selectObj(mText);
	ctrl()->hideObj(mText);
	mState = SELECTED;
	emit overlayChanged();
}

void TextEditor::finishEdit()
{
	int width = mDialog->textWidth();
	if (!mDialog->isWidthSet())
		width = 0.1 * mDialog->textHeight();
	TextEditCmd *cmd = new TextEditCmd(NULL, mText, mPos, mDialog->layer(),
									   (mText->angle() + mAngleDelta) % 360, mDialog->isMirrored(), mDialog->isNegative(),
									   mDialog->textHeight(), width, mDialog->text());
	ctrl()->doc()->doCommand(cmd);
	emit overlayChanged();
}

void TextEditor::drawOverlay(QPainter *painter)
{
	if (!mText) return;
	if (mState == SELECTED)
	{
		mText->draw(painter, Layer::LAY_SELECTION);
		painter->save();
		painter->setBrush(Qt::NoBrush);
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->drawRect(mText->bbox());
		painter->restore();
	}
	else
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->setBrush(Qt::NoBrush);
		painter->translate(mPos);
		painter->drawLine(QPoint(0, -INT_MAX), QPoint(0, INT_MAX));
		painter->drawLine(QPoint(-INT_MAX, 0), QPoint(INT_MAX, 0));
		painter->rotate(mAngleDelta);
		painter->drawRect(mBox);
		painter->restore();
	}
}

//////////////////////////// UNDO COMMANDS ///////////////////////////////////////

TextMoveCmd::TextMoveCmd(QUndoCommand *parent, Text *obj, QPoint newPos, int angleDelta)
	: QUndoCommand(parent), mText(obj), mNewPos(newPos), mPrevPos(obj->pos()),
	mNewAngle((obj->angle() + angleDelta) % 360), mPrevAngle(obj->angle())
{
	setText("move text");
}

void TextMoveCmd::undo()
{
	mText->setPos(mPrevPos);
	mText->setAngle(mPrevAngle);
}

void TextMoveCmd::redo()
{
	mText->setPos(mNewPos);
	mText->setAngle(mNewAngle);
}

TextNewCmd::TextNewCmd(QUndoCommand *parent, Text *obj, Document *doc)
	:QUndoCommand(parent), mText(obj), mDoc(doc), mInDoc(false)
{
	setText("add text");
}

TextNewCmd::~TextNewCmd()
{
	if(!mInDoc)
		delete mText;
}

void TextNewCmd::redo()
{
	mInDoc = true;
	mDoc->addText(mText);
}

void TextNewCmd::undo()
{
	mInDoc = false;
	mDoc->removeText(mText);
}

TextDeleteCmd::TextDeleteCmd(QUndoCommand *parent, Text *obj, Document *doc)
	:QUndoCommand(parent), mText(obj), mDoc(doc), mInDoc(true)
{
	setText("delete text");
}

TextDeleteCmd::~TextDeleteCmd()
{
	if(!mInDoc)
		delete mText;
}

void TextDeleteCmd::redo()
{
	mInDoc = false;
	mDoc->removeText(mText);
}

void TextDeleteCmd::undo()
{
	mInDoc = true;
	mDoc->addText(mText);
}

TextEditCmd::TextEditCmd(QUndoCommand *parent, Text* obj, QPoint newPos, const Layer& newLayer,
			int newAngle, bool isMirrored, bool isNegative, int newSize,
			int newWidth, QString newText )
				: QUndoCommand(parent), mText(obj), mOldPos(obj->pos()), mOldLayer(obj->layer()),
				mOldAngle(obj->angle()), mOldIsMirrored(obj->isMirrored()),
				mOldIsNegative(obj->isNegative()), mOldFontSize(obj->fontSize()),
				mOldStrokeWidth(obj->strokeWidth()), mOldText(obj->text()),
				mNewPos(newPos), mNewLayer(newLayer), mNewAngle(newAngle), mNewIsMirrored(isMirrored),
				mNewIsNegative(isNegative), mNewFontSize(newSize), mNewStrokeWidth(newWidth), mNewText(newText)
{
	setText("edit text");
}

void TextEditCmd::undo()
{
	mText->setPos(mOldPos);
	mText->setLayer(mOldLayer);
	mText->setAngle(mOldAngle);
	mText->setMirrored(mOldIsMirrored);
	mText->setNegative(mOldIsNegative);
	mText->setFontSize(mOldFontSize);
	mText->setStrokeWidth(mOldStrokeWidth);
	mText->setText(mOldText);
}

void TextEditCmd::redo()
{
	mText->setPos(mNewPos);
	mText->setLayer(mNewLayer);
	mText->setAngle(mNewAngle);
	mText->setMirrored(mNewIsMirrored);
	mText->setNegative(mNewIsNegative);
	mText->setFontSize(mNewFontSize);
	mText->setStrokeWidth(mNewStrokeWidth);
	mText->setText(mNewText);
}
