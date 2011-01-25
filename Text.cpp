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
	: PCBObject(), mLayer(LAY_UNKNOWN), mAngle(0), mIsMirrored(false), mIsNegative(false),
	mFontSize(0), mStrokeWidth(0), mIsDirty(true)
{
}

Text::Text( const QPoint &pos, int angle, bool mirror,
			bool negative, PCBLAYER layer, int font_size, int stroke_width,
			const QString &str ) :
PCBObject(), mPos(pos), mLayer(layer), mAngle(angle), mIsMirrored(mirror), mIsNegative(negative),
mFontSize(font_size), mStrokeWidth(stroke_width), mText(str), mIsDirty(true)
{

}

Text::~Text()
{
}

void Text::initText() const
{
	double scale = (double)mFontSize/22.0;
	QRect newBbox;
	SMFontUtil &fu = SMFontUtil::instance();

	mStrokes.clear();

	// get new strokes for all characters
	for (int i = 0; i < mText.size(); i++)
	{
		if (mText[i] == ' ')
			newBbox.adjust(0, 0, 0.5*mFontSize, 0);
		else
			fu.GetCharPath(mText[i].toAscii(), SIMPLEX, QPoint(newBbox.right() + 1.5*mStrokeWidth, 0), scale, newBbox, mStrokes);
	}
	mStrokeBBox = newBbox.adjusted(-mStrokeWidth, -mStrokeWidth, mStrokeWidth, mStrokeWidth);
}

void Text::initTransform() const
{
//	double scale = (double)mFontSize/22.0;
	double yoffset = 9.0*mFontSize/22.0;
	mTransform.reset();
	mTransform.translate(mPos.x(), mPos.y());
	mTransform.rotate(mAngle);
//	mTransform.scale(scale, scale);
	mTransform.translate(0, yoffset);

}

QRect Text::bbox() const
{
	if (mIsDirty)
	{
		initText();
		initTransform();
		mIsDirty = false;
	}
	return mTransform.mapRect(this->mStrokeBBox);
}

void Text::draw(QPainter *painter, PCBLAYER layer) const
{
	if (layer != mLayer && layer != LAY_SELECTION)
		return;

	// reload strokes if text has changed
	if (mIsDirty)
	{
		initText();
		initTransform();
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
	t->mLayer = (PCBLAYER) attr.value("layer").toString().toInt();
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
	writer.writeAttribute("layer", QString::number(mLayer));
	writer.writeAttribute("x", QString::number(mPos.x()));
	writer.writeAttribute("y", QString::number(mPos.y()));
	writer.writeAttribute("rot", QString::number(mAngle));
	writer.writeAttribute("lineWidth", QString::number(mStrokeWidth));
	writer.writeAttribute("textSize", QString::number(mFontSize));
	writer.writeCharacters(mText);
	writer.writeEndElement();
}

/////////////////////////////////////////////////////////////////////////////
// EDITOR

TextEditor::TextEditor(Controller *ctrl, QList<QAction*> actions, Text *text)
	: AbstractEditor(ctrl, actions), mState(SELECTED), mText(text), mDialog(NULL), mAngleDelta(0)
{
	ctrl->hideObj(text);
	updateActions();
}

TextEditor::~TextEditor()
{
	if (mDialog)
		delete mDialog;
}

void TextEditor::updateActions()
{
	disconnect(mActions[0], 0, this, 0);
	disconnect(mActions[2], 0, this, 0);
	disconnect(mActions[3], 0, this, 0);
	disconnect(mActions[7], 0, this, 0);
	for(int i = 0; i < 8; i++)
	{
		mActions[i]->setVisible(false);
	}
	switch(mState)
	{
	case MOVE:
	case ADD_MOVE:
	case EDIT_MOVE:
		mActions[2]->setText("Rotate Text");
		mActions[2]->setVisible(true);
		mActions[2]->setEnabled(true);
		connect(mActions[2], SIGNAL(triggered()), this, SLOT(actionRotate()));
		break;
	case SELECTED:
		mActions[0]->setText("Edit Text");
		mActions[0]->setVisible(true);
		mActions[0]->setEnabled(true);
		connect(mActions[0], SIGNAL(triggered()), this, SLOT(actionEdit()));
		mActions[3]->setText("Move Text");
		mActions[3]->setVisible(true);
		mActions[3]->setEnabled(true);
		connect(mActions[3], SIGNAL(triggered()), this, SLOT(actionMove()));
		mActions[7]->setText("Delete Text");
		mActions[7]->setVisible(true);
		mActions[7]->setEnabled(true);
		connect(mActions[7], SIGNAL(triggered()), this, SLOT(actionDelete()));
		break;
	}
}

bool TextEditor::eventFilter(QObject *watched, QEvent *event)
{
	event->accept();
	if (event->type() == QEvent::MouseMove)
	{
		mouseMoveEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonPress)
	{
		mousePressEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		mouseReleaseEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::KeyPress)
	{
		keyPressEvent(static_cast<QKeyEvent*>(event));
	}
	else
	{
		event->ignore();
		return false;
	}
	return event->isAccepted();
}

void TextEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		mPos = mCtrl->snapToPlaceGrid(mCtrl->view()->transform().inverted().map(event->pos()));
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
		updateActions();
		TextMoveCmd* cmd = new TextMoveCmd(NULL, mText, mPos, mAngleDelta);
		mCtrl->doc()->doCommand(cmd);
		emit overlayChanged();
	}
	else if (mState == EDIT_MOVE)
	{
		event->accept();
		mState = SELECTED;
		updateActions();
		finishEdit();
	}
	else
		event->ignore();
}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState != SELECTED)
	{
		mState = SELECTED;
		updateActions();
		emit overlayChanged();
		event->accept();
	}
	else
		event->ignore();
}

void TextEditor::actionMove()
{

	mState = MOVE;
	startMove();
	QCursor::setPos(mCtrl->view()->mapToGlobal(mCtrl->view()->transform().map(mPos)));
	updateActions();
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
	TextDeleteCmd* cmd = new TextDeleteCmd(NULL, mText, mCtrl->doc());
	mCtrl->doc()->doCommand(cmd);
	emit editorFinished();
}

void TextEditor::actionRotate()
{
	mAngleDelta += 90;
	mAngleDelta %= 360;
	emit overlayChanged();
}

void TextEditor::actionEdit()
{
	if (!mDialog)
		mDialog = new EditTextDialog();
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
			updateActions();
			emit overlayChanged();
		}
	}
}

void TextEditor::finishEdit()
{
	TextEditCmd *cmd = new TextEditCmd(NULL, mText, mPos, mDialog->layer(),
									   (mText->angle() + mAngleDelta) % 360, mDialog->isMirrored(), mDialog->isNegative(),
									   mDialog->textHeight(), mDialog->textWidth(), mDialog->text());
	mCtrl->doc()->doCommand(cmd);
	emit overlayChanged();
}

void TextEditor::drawOverlay(QPainter *painter)
{
	if (mState == SELECTED)
	{
		mText->draw(painter, LAY_SELECTION);
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->drawRect(mText->bbox());
		painter->restore();
	}
	else
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
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

TextNewCmd::TextNewCmd(QUndoCommand *parent, Text *obj, PCBDoc *doc)
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

TextDeleteCmd::TextDeleteCmd(QUndoCommand *parent, Text *obj, PCBDoc *doc)
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

TextEditCmd::TextEditCmd(QUndoCommand *parent, Text* obj, QPoint newPos, PCBLAYER newLayer,
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
