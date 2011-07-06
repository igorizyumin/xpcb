/*
	Copyright (C) 2010-2011 Igor Izyumin

	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMouseEvent>
#include "Text.h"
#include "Log.h"
#include "smfontutil.h"
#include "Controller.h"
#include "PCBView.h"
#include "Document.h"
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

QSharedPointer<Text> Text::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "text");

	QXmlStreamAttributes attr = reader.attributes();
	QSharedPointer<Text> t(new Text());
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

PCBObjState Text::getState() const
{
	return PCBObjState(new TextState(*this));
}


bool Text::loadState(PCBObjState &state)
{
	// convert to text state
	QSharedPointer<TextState> s = state.ptr().dynamicCast<TextState>();
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

TextEditor::TextEditor(Controller *ctrl, QSharedPointer<Text> text)
		: AbstractEditor(ctrl), mState(SELECTED), mText(text), mAngleDelta(0),
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
		PCBObjState s = mText->getState();
		mText->setPos(mPos);
		mText->setAngle((mText->angle() + mAngleDelta) % 360);
		PCBObjEditCmd* cmd = new PCBObjEditCmd(NULL, mText, s);
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
			mText.clear();
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
	if (mDialog.isNull())
		mDialog = QSharedPointer<EditTextDialog>(new EditTextDialog(ctrl()->view()));
	mDialog->init();
	if (mDialog->exec() == QDialog::Rejected || mDialog->text().isEmpty())
	{
		emit editorFinished();
		return;
	}
	mText = QSharedPointer<Text>(new Text(mDialog->pos(), mDialog->angle(), mDialog->isMirrored(), mDialog->isNegative(),
					 mDialog->layer(), mDialog->textHeight(), mDialog->textWidth(), mDialog->text()));
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
	if (mDialog.isNull())
		mDialog = QSharedPointer<EditTextDialog>(new EditTextDialog(ctrl()->view()));
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
	PCBObjState s = mText->getState();
	mText->setPos(mPos);
	mText->setLayer(mDialog->layer());
	mText->setAngle((mText->angle() + mAngleDelta) % 360);
	mText->setMirrored(mDialog->isMirrored());
	mText->setNegative(mDialog->isNegative());
	mText->setFontSize(mDialog->textHeight());
	mText->setStrokeWidth(width);
	mText->setText(mDialog->text());
	PCBObjEditCmd *cmd = new PCBObjEditCmd(NULL, mText, s);
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


TextNewCmd::TextNewCmd(QUndoCommand *parent, QSharedPointer<Text> obj, Document *doc)
	:QUndoCommand(parent), mText(obj), mDoc(doc), mInDoc(false)
{
	setText("add text");
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

TextDeleteCmd::TextDeleteCmd(QUndoCommand *parent, QSharedPointer<Text> obj, Document *doc)
	:QUndoCommand(parent), mText(obj), mDoc(doc), mInDoc(true)
{
	setText("delete text");
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
