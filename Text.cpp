#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMouseEvent>
#include "Text.h"
#include "Log.h"
#include "smfontutil.h"
#include "Controller.h"
#include "PCBView.h"
#include "PCBDoc.h"

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
	double scale = (double)mFontSize/22.0;
	double yoffset = 9.0*scale;
	mTransform.reset();
	mTransform.translate(mPos.x(), mPos.y());
	mTransform.rotate(mAngle);
	mTransform.scale(scale, scale);
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

Text Text::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "text");

	QXmlStreamAttributes attr = reader.attributes();
	Text t;
	t.mLayer = (PCBLAYER) attr.value("layer").toString().toInt();
	t.mPos = QPoint(
			attr.value("x").toString().toInt(),
			attr.value("y").toString().toInt());
	t.mAngle = attr.value("rot").toString().toInt();
	t.mStrokeWidth = attr.value("lineWidth").toString().toInt();
	t.mFontSize = attr.value("textSize").toString().toInt();
	t.mText = reader.readElementText();
	t.mIsDirty = true;

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
	: AbstractEditor(ctrl, actions), mIsMoving(false), mText(text), mAngleDelta(0)
{
	ctrl->hideObj(text);
	updateActions();
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
	if (mIsMoving)
	{
		mActions[2]->setText("Rotate Text");
		mActions[2]->setVisible(true);
		mActions[2]->setEnabled(true);
		connect(mActions[2], SIGNAL(triggered()), this, SLOT(actionRotate()));
	}
	else
	{
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
	if (mIsMoving)
	{
		mPos = mCtrl->snapToPlaceGrid(mCtrl->view()->transform().inverted().map(event->pos()));
		emit overlayChanged();

	}

	// we don't want to eat mouse events
	event->ignore();
}

void TextEditor::mousePressEvent(QMouseEvent *event)
{
	if (mIsMoving)
		event->accept();
	else
		event->ignore();
}

void TextEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mIsMoving)
	{
		event->accept();
		mIsMoving = false;
		updateActions();
		TextMoveCmd* cmd = new TextMoveCmd(NULL, mText, mPos, mAngleDelta);
		mCtrl->doc()->doCommand(cmd);
		emit overlayChanged();
	}
	else
		event->ignore();
}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mIsMoving)
	{
		mIsMoving = false;
		updateActions();
		emit overlayChanged();
		event->accept();
	}
	else
		event->ignore();
}

void TextEditor::actionMove()
{
	mIsMoving = true;
	mPos = mText->pos();
	QCursor::setPos(mCtrl->view()->mapToGlobal(mCtrl->view()->transform().map(mPos)));
	mAngleDelta = 0;
	mBox = mText->bbox().translated(-mPos);
	updateActions();
	emit overlayChanged();
}

void TextEditor::actionDelete()
{
}

void TextEditor::actionRotate()
{
	mAngleDelta += 90;
	mAngleDelta %= 360;
	emit overlayChanged();
}

void TextEditor::actionEdit()
{
}

void TextEditor::drawOverlay(QPainter *painter)
{
	if (!mIsMoving)
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
