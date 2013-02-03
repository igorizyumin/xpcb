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

#include "Document.h"
#include "Log.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#include <QSettings>
#include "Polygon.h"
#include "Area.h"
#include "Trace.h"
#include "Line.h"

////////////// DOCUMENT ////////////////////////////////////////////////

Document::Document()
	: mUnits(XPcb::MM), mUndoStack(this)
{
	connect(&mUndoStack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
	connect(&mUndoStack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
	connect(&mUndoStack, SIGNAL(cleanChanged(bool)), this, SIGNAL(cleanChanged(bool)));
}

Document::~Document()
{
	mUndoStack.clear();
}

bool Document::isModified()
{
	return !mUndoStack.isClean();
}

void Document::doCommand(QUndoCommand *cmd)
{
	mUndoStack.push(cmd);
	emit changed();
}

void Document::undo()
{
	mUndoStack.undo();
	emit changed();
}

void Document::redo()
{
	mUndoStack.redo();
	emit changed();
}

bool Document::loadFromFile(const QString& path)
{
	QFile inFile(path);
	if (!inFile.open(QIODevice::ReadOnly))
	{
		Log::instance().error(QString("Unable to open file %1 for reading").arg(path));
		return false;
	}
	bool ret = loadFromFile(inFile);
	inFile.close();
	return ret;
}

bool Document::saveToFile(const QString &file)
{
	QFile outFile(file);
	if (!outFile.open(QIODevice::WriteOnly))
	{
		Log::instance().error(QString("Unable to open file %1 for writing").arg(file));
		return false;
	}

	QXmlStreamWriter writer(&outFile);

	bool ret = saveToXml(writer);

	outFile.close();
	return ret;
}

/////////////////////////////// FPDOC /////////////////////////////////

FPDoc::FPDoc()
	: mFp(QSharedPointer<Footprint>(new Footprint()))
{
}

QList<Layer> FPDoc::layerList(LayerOrder order, Document::LayerMask mask)
{
	QList<Layer> l = QList<Layer>();

	if (order == ListOrder)
	{
		if (mask & Document::Display)
		{
			l.append(Layer(Layer::LAY_SELECTION));
			l.append(Layer(Layer::LAY_VISIBLE_GRID));
			l.append(Layer(Layer::LAY_CENTROID));
		}
		if (mask & Document::Silk)
		{
			l.append(Layer(Layer::LAY_SILK_TOP));
			l.append(Layer(Layer::LAY_SILK_BOTTOM));
		}
		if (mask & Document::Glue)
			l.append(Layer(Layer::LAY_GLUE));
		if (mask & Document::Mask)
		{
			l.append(Layer(Layer::LAY_SMCUT_TOP));
			l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		}
		if (mask & Document::Paste)
		{
			l.append(Layer(Layer::LAY_PASTE_TOP));
			l.append(Layer(Layer::LAY_PASTE_BOTTOM));
		}
		if (mask & Document::Hole)
			l.append(Layer(Layer::LAY_HOLE));
		if (mask & Document::Copper)
		{
			l.append(Layer(Layer::LAY_START));
			l.append(Layer(Layer::LAY_INNER));
			l.append(Layer(Layer::LAY_END));
		}
	}
	else
	{
		if (mask & Document::Display)
		{
			l.append(Layer(Layer::LAY_VISIBLE_GRID));
			l.append(Layer(Layer::LAY_CENTROID));
		}
		if (mask & Document::Mask)
			l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		if (mask & Document::Paste)
			l.append(Layer(Layer::LAY_PASTE_BOTTOM));
		if (mask & Document::Silk)
			l.append(Layer(Layer::LAY_SILK_BOTTOM));
		if (mask & Document::Copper)
		{
			l.append(Layer(Layer::LAY_END));
			l.append(Layer(Layer::LAY_INNER));
			l.append(Layer(Layer::LAY_START));
		}
		if (mask & Document::Hole)
			l.append(Layer(Layer::LAY_HOLE));
		if (mask & Document::Mask)
			l.append(Layer(Layer::LAY_SMCUT_TOP));
		if (mask & Document::Paste)
			l.append(Layer(Layer::LAY_PASTE_TOP));
		if (mask & Document::Silk)
			l.append(Layer(Layer::LAY_SILK_TOP));
		if (mask & Document::Glue)
			l.append(Layer(Layer::LAY_GLUE));
		if (mask & Document::Display)
			l.append(Layer(Layer::LAY_SELECTION));
	}

	return l;
}

QList<QSharedPointer<PCBObject> > FPDoc::objects()
{
	Q_ASSERT(mFp && mFp->refText() && mFp->valueText());

	QList<QSharedPointer<PCBObject> > out;

	foreach(QSharedPointer<Pin> p, mFp->pins())
	{
		out.append(p);
	}
	foreach(QSharedPointer<Text> p, mFp->texts())
	{
		out.append(p);
	}
	foreach(QSharedPointer<Line> p, mFp->lines())
	{
		out.append(p);
	}

	out.append(mFp->refText());
	out.append(mFp->valueText());

	return out;
}

QList<QSharedPointer<PCBObject> > FPDoc::findObjs(QPoint &pt, int dist)
{
	Q_ASSERT(mFp && mFp->refText() && mFp->valueText());

	QList<QSharedPointer<PCBObject> > out;

	QRect hitRect(pt.x()-dist/2, pt.y()-dist/2, dist, dist);

	foreach(QSharedPointer<Pin> p, mFp->pins())
	{
		if (p->bbox().intersects(hitRect))
			out.append(p);
	}
	foreach(QSharedPointer<Text> p, mFp->texts())
	{
		if (p->bbox().intersects(hitRect))
			out.append(p);
	}
	foreach(QSharedPointer<Line> p, mFp->lines())
	{
		if (p->bbox().intersects(hitRect))
			out.append(p);
	}

	if (mFp->refText()->bbox().intersects(hitRect))
		out.append(mFp->refText());
	if (mFp->valueText()->bbox().intersects(hitRect))
		out.append(mFp->valueText());

	return out;
}

QList<QSharedPointer<PCBObject> > FPDoc::findObjs(QRect &rect)
{
	Q_ASSERT(mFp && mFp->refText() && mFp->valueText());

	QList<QSharedPointer<PCBObject> > out;

	QList<QSharedPointer<Pin> > pins = mFp->pins();
	QList<QSharedPointer<Line> > lines = mFp->lines();
	QList<QSharedPointer<Text> > texts = mFp->texts();

	foreach(QSharedPointer<Pin> p, pins)
	{
		if (p->bbox().intersects(rect))
			out.append(p);
	}
	foreach(QSharedPointer<Text> p, texts)
	{
		if (p->bbox().intersects(rect))
			out.append(p);
	}
	foreach(QSharedPointer<Line> p, lines)
	{
		if (p->bbox().intersects(rect))
			out.append(p);
	}
	if (mFp->refText()->bbox().intersects(rect))
		out.append(mFp->refText());
	if (mFp->valueText()->bbox().intersects(rect))
		out.append(mFp->valueText());

	return out;
}

////////////////////////////// PCBDOC /////////////////////////////////

PCBDoc::PCBDoc()
		: mNumLayers(2), mTraceList(new TraceList(this)),
		  mNetlist(new Netlist())
{
}

QSharedPointer<Footprint> PCBDoc::getFootprint(QUuid uuid)
{
	// first see if we already have it in the cache
	if (mFootprints.contains(uuid))
		return mFootprints.value(uuid);
	// otherwise get it from the db and cache it
	QSharedPointer<const FPDBFile> f = FPDatabase::instance().getByUuid(uuid);
	if (!f)
		return QSharedPointer<Footprint>();
	QSharedPointer<Footprint> ptr = f->loadFootprint();
	if (!ptr.isNull())
		mFootprints.insert(uuid, ptr);
	return ptr;
}

QSharedPointer<Part> PCBDoc::part(const QString &refdes) const
{
	foreach(QSharedPointer<Part> p, mParts)
	{
		if (p->refdes() == refdes)
			return p;
	}
	return QSharedPointer<Part>();
}

QList<QSharedPointer<PartPin> > PCBDoc::partPins() const
{
	QList<QSharedPointer<PartPin> > out;
	foreach(QSharedPointer<Part> p, mParts)
	{
		out.append(p->pins());
	}
	return out;
}

void PCBDoc::addText(QSharedPointer<Text> t)
{
	if (!mTexts.contains(t))
		mTexts.append(t);
	else
		Log::instance().error("Text object already in document");
}

void PCBDoc::removeText(QSharedPointer<Text> t)
{
	if (mTexts.contains(t))
	{
		mTexts.removeOne(t);
		Log::instance().message("Text object deleted");
	}
	else
		Log::instance().error("Text object does not exist in document");
}

QList<QSharedPointer<PCBObject> > PCBDoc::objects()
{
	QList<QSharedPointer<PCBObject> > out;

	// have to do this the inefficient way thanks to C++ retardation
	// (no way to convert QList<Pin*> to QList<PCBObject*>)
	foreach(QSharedPointer<Segment> s, mTraceList->segments())
	{
		out.append(s);
	}

	foreach(QSharedPointer<Vertex> v, mTraceList->vertices())
	{
		out.append(v);
	}

	foreach(QSharedPointer<Part> p, mParts)
	{
		out.append(p);
		out.append(p->refdesText());
		out.append(p->valueText());
	}

	foreach(QSharedPointer<Text> t, mTexts)
	{
		out.append(t);
	}

	foreach(QSharedPointer<Area> a, mAreas)
	{
		out.append(a);
	}

	return out;
}

QList<QSharedPointer<PCBObject> > PCBDoc::findObjs(QRect &rect)
{
	QList<QSharedPointer<PCBObject> > out;

	foreach(QSharedPointer<Segment> s, mTraceList->segments())
	{
		if (rect.intersects(s->bbox()))
			out.append(s);
	}

	foreach(QSharedPointer<Vertex> v, mTraceList->vertices())
	{
		if (rect.intersects(v->bbox()))
			out.append(v);
	}

	foreach(QSharedPointer<Part> p, mParts)
	{
		if (rect.intersects(p->bbox()))
			out.append(p);
		if (p->refVisible() && p->refdesText()->bbox().intersects(rect))
			out.append(p->refdesText());
		if (p->valueVisible() && p->valueText()->bbox().intersects(rect))
			out.append(p->valueText());
	}

	foreach(QSharedPointer<Text> t, mTexts)
	{
		if (rect.intersects(t->bbox()))
			out.append(t);
	}

	foreach(QSharedPointer<Area> a, mAreas)
	{
		if (rect.intersects(a->bbox()))
			out.append(a);
	}

	return out;
}

QList<QSharedPointer<PCBObject> > PCBDoc::findObjs(QPoint &pt, int dist)
{
	QList<QSharedPointer<PCBObject> > out;
	QRect hitRect(pt.x()-dist/2, pt.y()-dist/2, dist, dist);

	foreach(QSharedPointer<Part> p, mParts)
	{
		if (p->bbox().intersects(hitRect))
		{
			out.append(p);
		}
		if (p->refVisible() && p->refdesText()->bbox().intersects(hitRect))
			out.append(p->refdesText());
		if (p->valueVisible() && p->valueText()->bbox().intersects(hitRect))
			out.append(p->valueText());
	}
	foreach(QSharedPointer<Text> t, mTexts)
	{
		if(t->bbox().intersects(hitRect))
			out.append(t);
	}
	foreach(QSharedPointer<Segment> s, mTraceList->segments())
	{
		if (s->bbox().intersects(hitRect))
			out.append(s);
	}
	foreach(QSharedPointer<Vertex> v, mTraceList->vertices())
	{
		if (v->bbox().intersects(hitRect))
			out.append(v);
	}
    foreach(QSharedPointer<Area> a, mAreas)
    {
        if (a->bbox().intersects(hitRect))
            out.append(a);
    }

	return out;
}

void PCBDoc::clearDoc()
{
	mTraceList = QSharedPointer<TraceList>(new TraceList(this));

	mParts.clear();
	mTexts.clear();
	mAreas.clear();
	mFootprints.clear();
	mPadstacks.clear();

	mBoardOutline = Polygon();

	mDefaultPadstack = QUuid();
}

QList<Layer> PCBDoc::layerList(LayerOrder order, Document::LayerMask mask)
{
	if (order == ListOrder)
	{
		QList<Layer> l = QList<Layer>();
		if (mask & Document::Display)
		{
			l.append(Layer(Layer::LAY_SELECTION));
			l.append(Layer(Layer::LAY_VISIBLE_GRID));
			l.append(Layer(Layer::LAY_DRC));
			l.append(Layer(Layer::LAY_BOARD_OUTLINE));
			l.append(Layer(Layer::LAY_RAT_LINE));
		}
		if (mask & Document::Silk)
		{
			l.append(Layer(Layer::LAY_SILK_TOP));
			l.append(Layer(Layer::LAY_SILK_BOTTOM));
		}
		if (mask & Document::Mask)
		{
			l.append(Layer(Layer::LAY_SMCUT_TOP));
			l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		}
		if (mask & Document::Hole)
			l.append(Layer(Layer::LAY_HOLE));
		if (mask & Document::Copper)
		{
			l.append(Layer(Layer::LAY_TOP_COPPER));
			for(int i = 0; i < numLayers() - 2; i++)
				l.append(Layer((Layer::Type(Layer::LAY_INNER1+i))));
			l.append(Layer(Layer::LAY_BOTTOM_COPPER));
		}
		return l;
	}
	else
	{
		QList<Layer> l = QList<Layer>();
		if (mask & Document::Display)
			l.append(Layer(Layer::LAY_VISIBLE_GRID));
		if (mask & Document::Copper)
		{
			l.append(Layer(Layer::LAY_BOTTOM_COPPER));
			for(int i = numLayers() - 3; i >= 0; i--)
				l.append(Layer(Layer::Type(Layer::LAY_INNER1+i)));
			l.append(Layer(Layer::LAY_TOP_COPPER));
		}
		if (mask & Document::Hole)
			l.append(Layer(Layer::LAY_HOLE));
		if (mask & Document::Mask)
		{
			l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
			l.append(Layer(Layer::LAY_SMCUT_TOP));
		}
		if (mask & Document::Silk)
		{
			l.append(Layer(Layer::LAY_SILK_BOTTOM));
			l.append(Layer(Layer::LAY_SILK_TOP));
		}
		if (mask & Document::Display)
		{
			l.append(Layer(Layer::LAY_RAT_LINE));
			l.append(Layer(Layer::LAY_BOARD_OUTLINE));
			l.append(Layer(Layer::LAY_DRC));
			l.append(Layer(Layer::LAY_SELECTION));
		}
		return l;
	}
}

void PCBDoc::addPadstack(QSharedPointer<Padstack> ps)
{
	mPadstacks.insert(ps->uuid(), ps);
}

void PCBDoc::removePadstack(QSharedPointer<Padstack> ps)
{
	mPadstacks.remove(ps->uuid());
}

QSharedPointer<Padstack> PCBDoc::padstack(QUuid uuid)
{
	if (mPadstacks.contains(uuid))
		return mPadstacks[uuid];
	return QSharedPointer<Padstack>();
}

void PCBDoc::addPart(QSharedPointer<Part> p)
{
	Q_ASSERT(!mParts.contains(p));
	mParts.append(p);
	emit partsChanged();
}

void PCBDoc::removePart(QSharedPointer<Part> p)
{
	Q_ASSERT(mParts.contains(p));
	mParts.removeOne(p);
	emit partsChanged();
}

void PCBDoc::addArea(QSharedPointer<Area> a)
{
	Q_ASSERT(!mAreas.contains(a));
	mAreas.append(a);
}

void PCBDoc::removeArea(QSharedPointer<Area> a)
{
	Q_ASSERT(mAreas.contains(a));
	mAreas.removeOne(a);
}

//////// XML PARSING /////////
// parser methods
static void loadProps(QXmlStreamReader &reader, QString &name,
					  XPcb::UNIT &units, QUuid &defaultps, int &numLayers);
static void loadPadstacks(QXmlStreamReader &reader,
						  QHash<QUuid, QSharedPointer<Padstack> > &padstacks);
static void loadFootprints(QXmlStreamReader &reader, QHash<QUuid,
						   QSharedPointer<Footprint> > &footprints);
static void loadOutline(QXmlStreamReader &reader, Polygon & poly);
static void loadParts(QXmlStreamReader &reader,
					  QList<QSharedPointer<Part> >& parts, PCBDoc* doc);
static void loadAreas(QXmlStreamReader &reader,
					  QList<QSharedPointer<Area> >& areas, PCBDoc *doc);
static void loadTexts(QXmlStreamReader &reader,
					  QList<QSharedPointer<Text> >& texts);


bool validateFile(QIODevice &file, const QUrl &uri)
{
	QFile schemaFile(":/xpcbschema.xsd");
	if (!schemaFile.open(QIODevice::ReadOnly))
	{
		Log::instance().error(QString("Error loading XML schema: %1").arg(schemaFile.errorString()));
		return false;
	}
	QXmlSchema schema;
	bool result = schema.load(&schemaFile);
	if (!result)
	{
		Log::instance().error("Error reading XML schema");
		return false;
	}
	schemaFile.close();
	QXmlSchemaValidator validator;
	validator.setSchema(schema);
	return validator.validate(&file, uri);
}

bool PCBDoc::saveToXml(QXmlStreamWriter &writer)
{
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("xpcbBoard");

	writer.writeStartElement("props");
	writer.writeTextElement("units", this->mUnits == XPcb::MM ? "mm" : "mils");
	writer.writeTextElement("numLayers", QString::number(this->mNumLayers));
	writer.writeTextElement("name", this->mName);
	writer.writeTextElement("defaultPadstack",
							mDefaultPadstack.toString());
	writer.writeEndElement();

	writer.writeStartElement("padstacks");
	foreach(QSharedPointer<Padstack> ps, mPadstacks)
		ps->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("footprints");
	foreach(QSharedPointer<Footprint> fp, mFootprints)
		fp->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("outline");
	this->mBoardOutline.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("parts");
	foreach(QSharedPointer<Part> p, mParts)
		p->toXML(writer);
	writer.writeEndElement();

	mNetlist->toXML(writer);

	mTraceList->toXML(writer);

	writer.writeStartElement("areas");
	foreach(QSharedPointer<Area> a, mAreas)
		a->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("texts");
	foreach(QSharedPointer<Text> t, mTexts)
		t->toXML(writer);
	writer.writeEndElement();

	writer.writeEndElement();
	writer.writeEndDocument();

	mUndoStack.setClean();
	return true;
}

bool PCBDoc::loadFromFile(QFile &inFile)
{
	if (!validateFile(inFile, QUrl(inFile.fileName())))
	{
		Log::instance().error("Unable to load file: XML validation failed");
		inFile.close();
		return false;
	}

	inFile.reset();
	QXmlStreamReader reader(&inFile);

	return loadFromXml(reader);
}

bool PCBDoc::loadFromXml(QXmlStreamReader &reader)
{
	clearDoc();
	mTraceList = QSharedPointer<TraceList>(new TraceList(this));
	mNetlist = QSharedPointer<Netlist>(new Netlist());

	reader.readNextStartElement();
	if (reader.name() != "xpcbBoard")
	{
		Log::instance().error("Not a PCB document.");
		return false;
	}
	while(reader.readNextStartElement())
	{
		QStringRef t = reader.name();
		if (t == "props")
			loadProps(reader, mName, mUnits, mDefaultPadstack, mNumLayers);
		else if (t == "padstacks")
		{
			loadPadstacks(reader, mPadstacks);
		}
		else if (t == "footprints")
			loadFootprints(reader, this->mFootprints);
		else if (t == "outline")
			loadOutline(reader, this->mBoardOutline);
		else if (t == "parts")
			loadParts(reader, this->mParts, this);
		else if (t == "netlist")
		{
			mNetlist->loadFromXML(reader);
		}
		else if (t == "traces")
		{
			mTraceList->loadFromXml(reader);
			do
					reader.readNext();
			while(!reader.isEndElement());
			Q_ASSERT(reader.isEndElement() && reader.name() == "traces");
		}
		else if (t == "areas")
			loadAreas(reader, this->mAreas, this);
		else if (t == "texts")
			loadTexts(reader, this->mTexts);
	}
	if (reader.hasError())
	{
		Log::instance().error(QString("Unable to load file: %1").arg(reader.errorString()));
		return false;
	}
	return true;
}

bool FPDoc::loadFromFile(QFile &file)
{
	if (!validateFile(file, QUrl(file.fileName())))
	{
		Log::instance().error("Unable to load file: XML validation failed");
		file.close();
		return false;
	}

	file.reset();
	QXmlStreamReader reader(&file);

	return loadFromXml(reader);
}

bool FPDoc::loadFromXml(QXmlStreamReader &reader)
{
	reader.readNextStartElement();
	if (reader.name() != "xpcbFootprint")
	{
		Log::instance().error("Not a footprint.");
		return false;
	}
	while(reader.readNextStartElement())
	{
		QStringRef t = reader.name();

		if (t == "footprint")
		{
			mFp = Footprint::newFromXML(reader);
			if (!mFp)
			{
				Log::instance().error("Error loading footprint");
				return false;
			}
		}
	}
	if (reader.hasError())
	{
		Log::instance().error(QString("Unable to load file: %1").arg(reader.errorString()));
		return false;
	}
	return true;
}

bool FPDoc::saveToXml(QXmlStreamWriter &writer)
{
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("xpcbFootprint");

	mFp->toXML(writer);

	writer.writeEndElement();
	writer.writeEndDocument();

	mUndoStack.setClean();
	return true;
}

void loadProps(QXmlStreamReader &reader, QString &name, XPcb::UNIT &units,
			   QUuid &defaultps, int &numLayers)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "props");
	while(reader.readNextStartElement())
	{
		QStringRef t = reader.name();
		if (t == "name")
		{
			name = reader.readElementText();
		}
		else if (t == "units")
		{
			QString unitsStr = reader.readElementText();
			if (unitsStr == "mm")
				units = XPcb::MM;
			else if (unitsStr == "mils")
				units = XPcb::MIL;
		}
		else if (t == "defaultPadstack")
		{
			defaultps = QUuid(reader.readElementText());
		}
		else if (t == "numLayers")
		{
			numLayers = reader.readElementText().toInt();
		}
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "props");
}

void loadPadstacks(QXmlStreamReader &reader,
				   QHash<QUuid, QSharedPointer<Padstack> > &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "padstacks");
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of padstacks block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "padstack")
		{
			QSharedPointer<Padstack> ps = Padstack::newFromXML(reader);
			if (!ps)
			{
				Log::instance().error("Error loading padstack");
			}
			else
				padstacks.insert(ps->uuid(), ps);
		}
	}
	do
			reader.readNext();
	while(!reader.isEndElement());
	Q_ASSERT(reader.isEndElement() && reader.name() == "padstacks");

}

void loadFootprints(QXmlStreamReader &reader, QHash<QUuid, QSharedPointer<Footprint> > &footprints)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "footprints");
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of footprints block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "footprint")
		{
			QSharedPointer<Footprint> fp = Footprint::newFromXML(reader);
			if (!fp)
			{
				Log::instance().error("Error loading footprint");
			}
			else
				footprints.insert(fp->uuid(), QSharedPointer<Footprint>(fp));
		}
	}
	do
			reader.readNext();
	while(!reader.isEndElement());
	Q_ASSERT(reader.isEndElement() && reader.name() == "footprints");

}

void loadOutline(QXmlStreamReader &reader, Polygon& poly)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "outline");
	if (!reader.readNextStartElement())
		return;
	poly = Polygon::newFromXML(reader);
	do
			reader.readNext();
	while(!reader.isEndElement());
	Q_ASSERT(reader.isEndElement() && reader.name() == "outline");

}

void loadParts(QXmlStreamReader &reader, QList<QSharedPointer<Part> >& parts, PCBDoc *doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "parts");
	while(reader.readNextStartElement())
	{
		QSharedPointer<Part> part = Part::newFromXML(reader, doc);
		if (!part)
		{
			Log::instance().error("Error loading part");
		}
		else
			parts.append(part);
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "parts");

}

void loadAreas(QXmlStreamReader &reader, QList<QSharedPointer<Area> > &areas, PCBDoc *doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "areas");
	while(reader.readNextStartElement())
	{
		QSharedPointer<Area> area = Area::newFromXML(reader, *doc);
		if (!area)
		{
			Log::instance().error("Error loading area");
		}
		else
			areas.append(area);
		do
				reader.readNext();
		while(!reader.isEndElement());
	}

	Q_ASSERT(reader.isEndElement() && reader.name() == "areas");

}

void loadTexts(QXmlStreamReader &reader, QList<QSharedPointer<Text> > &texts)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "texts");
	while(reader.readNextStartElement())
	{
		texts.append(QSharedPointer<Text>(Text::newFromXML(reader)));
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "texts");

}



