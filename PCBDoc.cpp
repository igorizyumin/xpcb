#include "PCBDoc.h"
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

QList<Layer> FPDoc::layerList(LayerOrder order)
{
	QList<Layer> l = QList<Layer>();

	if (order == ListOrder)
	{
		l.append(Layer(Layer::LAY_SELECTION));
		l.append(Layer(Layer::LAY_VISIBLE_GRID));
		l.append(Layer(Layer::LAY_CENTROID));
		l.append(Layer(Layer::LAY_SILK_TOP));
		l.append(Layer(Layer::LAY_SILK_BOTTOM));
		l.append(Layer(Layer::LAY_GLUE));
		l.append(Layer(Layer::LAY_SMCUT_TOP));
		l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		l.append(Layer(Layer::LAY_PASTE_TOP));
		l.append(Layer(Layer::LAY_PASTE_BOTTOM));
		l.append(Layer(Layer::LAY_HOLE));
		l.append(Layer(Layer::LAY_START));
		l.append(Layer(Layer::LAY_INNER));
		l.append(Layer(Layer::LAY_END));
	}
	else
	{
		l.append(Layer(Layer::LAY_VISIBLE_GRID));
		l.append(Layer(Layer::LAY_CENTROID));
		l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		l.append(Layer(Layer::LAY_PASTE_BOTTOM));
		l.append(Layer(Layer::LAY_SILK_BOTTOM));
		l.append(Layer(Layer::LAY_END));
		l.append(Layer(Layer::LAY_INNER));
		l.append(Layer(Layer::LAY_START));
		l.append(Layer(Layer::LAY_HOLE));
		l.append(Layer(Layer::LAY_SMCUT_TOP));
		l.append(Layer(Layer::LAY_PASTE_TOP));
		l.append(Layer(Layer::LAY_SILK_TOP));
		l.append(Layer(Layer::LAY_GLUE));
		l.append(Layer(Layer::LAY_SELECTION));
	}

	return l;
}

QList<QSharedPointer<PCBObject> > FPDoc::findObjs(QPoint &pt)
{
	Q_ASSERT(!mFp.isNull());

	QList<QSharedPointer<PCBObject> > out;

	foreach(QSharedPointer<Pin> p, mFp->pins())
	{
		if (p->bbox().contains(pt))
			out.append(p);
	}
	foreach(QSharedPointer<Text> p, mFp->texts())
	{
		if (p->bbox().contains(pt))
			out.append(p);
	}
	foreach(QSharedPointer<Line> p, mFp->lines())
	{
		if (p->bbox().contains(pt))
			out.append(p);
	}

	if (mFp->refText()->bbox().contains(pt))
		out.append(mFp->refText());
	if (mFp->valueText()->bbox().contains(pt))
		out.append(mFp->valueText());

	return out;
}

QList<QSharedPointer<PCBObject> > FPDoc::findObjs(QRect &rect)
{
	Q_ASSERT(!mFp.isNull());
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
		: mNumLayers(2), mTraceList(QSharedPointer<TraceList>(new TraceList())),
		  mDefaultPadstack(QSharedPointer<Padstack>(new Padstack()))
{
	mPadstacks.append(mDefaultPadstack);
}

PCBDoc::~PCBDoc()
{
}

QSharedPointer<Footprint> PCBDoc::getFootprint(QUuid uuid)
{
	// first see if we already have it in the cache
	if (mFootprints.contains(uuid))
		return mFootprints.value(uuid);
	// otherwise get it from the db and cache it
	const FPDBFile* f = FPDatabase::instance().getByUuid(uuid);
	if (!f) return QSharedPointer<Footprint>();
	QSharedPointer<Footprint> ptr = f->loadFootprint();
	if (!ptr.isNull())
		mFootprints.insert(uuid, ptr);
	return ptr;
}

QSharedPointer<Part> PCBDoc::part(const QString &refdes)
{
	foreach(QSharedPointer<Part> p, mParts)
	{
		if (p->refdes() == refdes)
			return p;
	}
	return QSharedPointer<Part>();
}

QSharedPointer<Net> PCBDoc::net(const QString &name) const
{
	foreach(QSharedPointer<Net> p, mNets)
	{
		if (p->name() == name)
			return p;
	}
	return QSharedPointer<Net>();
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

QList<QSharedPointer<PCBObject> > PCBDoc::findObjs(QRect &rect)
{
	QList<QSharedPointer<PCBObject> > out;

	foreach(QSharedPointer<Segment> s, mTraceList->segments())
	{
		if (rect.intersects(s->bbox()))
			out.append(s);
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

QList<QSharedPointer<PCBObject> > PCBDoc::findObjs(QPoint &pt)
{
	QList<QSharedPointer<PCBObject> > out;
	foreach(QSharedPointer<Part> p, mParts)
	{
		if (p->bbox().contains(pt))
		{
			out.append(p);
		}
		if (p->refVisible() && p->refdesText()->bbox().contains(pt))
			out.append(p->refdesText());
		if (p->valueVisible() && p->valueText()->bbox().contains(pt))
			out.append(p->valueText());
	}
	foreach(QSharedPointer<Text> t, mTexts)
	{
		if(t->bbox().contains(pt))
			out.append(t);
	}
	foreach(QSharedPointer<Segment> s, mTraceList->segments())
	{
		if (s->bbox().contains(pt))
			out.append(s);
	}

	return out;
}

void PCBDoc::clearDoc()
{
	mTraceList = QSharedPointer<TraceList>(new TraceList());

	mNets.clear();
	mParts.clear();
	mTexts.clear();
	mAreas.clear();
	mFootprints.clear();
	mPadstacks.clear();

	mBoardOutline = Polygon();

	mDefaultPadstack = QSharedPointer<Padstack>(new Padstack());
	mPadstacks.append(mDefaultPadstack);
}

QList<Layer> PCBDoc::layerList(LayerOrder order)
{
	if (order == ListOrder)
	{
		QList<Layer> l = QList<Layer>();
		l.append(Layer(Layer::LAY_SELECTION));
		l.append(Layer(Layer::LAY_VISIBLE_GRID));
		l.append(Layer(Layer::LAY_DRC));
		l.append(Layer(Layer::LAY_BOARD_OUTLINE));
		l.append(Layer(Layer::LAY_RAT_LINE));
		l.append(Layer(Layer::LAY_SILK_TOP));
		l.append(Layer(Layer::LAY_SILK_BOTTOM));
		l.append(Layer(Layer::LAY_SMCUT_TOP));
		l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		l.append(Layer(Layer::LAY_HOLE));
		l.append(Layer(Layer::LAY_TOP_COPPER));
		for(int i = 0; i < numLayers() - 2; i++)
			l.append(Layer((Layer::Type(Layer::LAY_INNER1+i))));
		l.append(Layer(Layer::LAY_BOTTOM_COPPER));
		return l;
	}
	else
	{
		QList<Layer> l = QList<Layer>();
		l.append(Layer(Layer::LAY_VISIBLE_GRID));
		l.append(Layer(Layer::LAY_BOTTOM_COPPER));
		for(int i = numLayers() - 3; i >= 0; i--)
			l.append(Layer(Layer::Type(Layer::LAY_INNER1+i)));
		l.append(Layer(Layer::LAY_TOP_COPPER));
		l.append(Layer(Layer::LAY_HOLE));
		l.append(Layer(Layer::LAY_SMCUT_BOTTOM));
		l.append(Layer(Layer::LAY_SMCUT_TOP));
		l.append(Layer(Layer::LAY_SILK_BOTTOM));
		l.append(Layer(Layer::LAY_SILK_TOP));
		l.append(Layer(Layer::LAY_RAT_LINE));
		l.append(Layer(Layer::LAY_BOARD_OUTLINE));
		l.append(Layer(Layer::LAY_DRC));
		l.append(Layer(Layer::LAY_SELECTION));
		return l;
	}
}

void PCBDoc::addPadstack(QSharedPointer<Padstack> ps)
{
	mPadstacks.append(ps);
}

void PCBDoc::removePadstack(QSharedPointer<Padstack> ps)
{
	mPadstacks.removeOne(ps);
}

void PCBDoc::addPart(QSharedPointer<Part> p)
{
	Q_ASSERT(!mParts.contains(p));
	mParts.append(p);
}

void PCBDoc::removePart(QSharedPointer<Part> p)
{
	Q_ASSERT(mParts.contains(p));
	mParts.removeOne(p);
}



//////// XML PARSING /////////
// parser methods
void loadProps(QXmlStreamReader &reader, QString &name, XPcb::UNIT &units, int &defaultps, int &numLayers);
void loadPadstacks(QXmlStreamReader &reader, QHash<int, QSharedPointer<Padstack> > &padstacks);
void loadFootprints(QXmlStreamReader &reader, QHash<QUuid, QSharedPointer<Footprint> > &footprints);
void loadOutline(QXmlStreamReader &reader, Polygon & poly);
void loadParts(QXmlStreamReader &reader, QList<QSharedPointer<Part> >& parts, PCBDoc* doc);
void loadNets(QXmlStreamReader &reader, QList<QSharedPointer<Net> >& nets, PCBDoc* doc, const QHash<int, QSharedPointer<Padstack> > &padstacks);
void loadAreas(QXmlStreamReader &reader, QList<QSharedPointer<Area> >& areas, PCBDoc *doc);
void loadTexts(QXmlStreamReader &reader, QList<QSharedPointer<Text> >& texts);


bool validateFile(QIODevice &file)
{
	const QString fileName = ":/xpcbschema.xsd";
	QFile schemaFile(fileName);
	if (!schemaFile.open(QIODevice::ReadOnly))
	{
		Log::instance().error(QString("Error loading XML schema: %1").arg(schemaFile.errorString()));
		return false;
	}
	QXmlSchema schema;
	bool result = schema.load(&schemaFile, QUrl::fromLocalFile(fileName));
	if (!result)
	{
		Log::instance().error("Error reading XML schema");
		return false;
	}
	schemaFile.close();
	QXmlSchemaValidator validator;
	validator.setSchema(schema);
	return validator.validate(&file);
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
	writer.writeTextElement("defaultPadstack", QString::number(this->mDefaultPadstack->getid()));
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

	writer.writeStartElement("nets");
	foreach(QSharedPointer<Net> n, mNets)
		n->toXML(writer);
	writer.writeEndElement();

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
	if (!validateFile(inFile))
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
	mTraceList = QSharedPointer<TraceList>(new TraceList());

	// default padstack id
	int defaultPS;

	QHash<int, QSharedPointer<Padstack> > padstacks;

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
			loadProps(reader, mName, mUnits, defaultPS, mNumLayers);
		else if (t == "padstacks")
		{
			loadPadstacks(reader, padstacks);
			mPadstacks.append(padstacks.values());
			if (padstacks.contains(defaultPS))
				mDefaultPadstack = padstacks.value(defaultPS);
			else
			{
				Log::instance().error("Invalid default padstack specified.");
				mDefaultPadstack = QSharedPointer<Padstack>(new Padstack());
				mPadstacks.append(mDefaultPadstack);
			}
		}
		else if (t == "footprints")
			loadFootprints(reader, this->mFootprints);
		else if (t == "outline")
			loadOutline(reader, this->mBoardOutline);
		else if (t == "parts")
			loadParts(reader, this->mParts, this);
		else if (t == "nets")
			loadNets(reader, this->mNets, this, padstacks);
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
	if (!validateFile(file))
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

void loadProps(QXmlStreamReader &reader, QString &name, XPcb::UNIT &units, int &defaultps, int &numLayers)
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
			defaultps = reader.readElementText().toInt();
		}
		else if (t == "numLayers")
		{
			numLayers = reader.readElementText().toInt();
		}
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "props");
}

void loadPadstacks(QXmlStreamReader &reader, QHash<int, QSharedPointer<Padstack> > &padstacks)
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
			int psid = reader.attributes().value("id").toString().toInt();
			QSharedPointer<Padstack> ps = Padstack::newFromXML(reader);
			if (!ps)
			{
				Log::instance().error("Error loading padstack");
			}
			else
				padstacks.insert(psid, ps);
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

void loadNets(QXmlStreamReader &reader, QList<QSharedPointer<Net> > &nets,
			  PCBDoc *doc,
			  const QHash<int, QSharedPointer<Padstack> > &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "nets");
	while(reader.readNextStartElement())
	{
		QSharedPointer<Net> net = Net::newFromXML(reader, doc, padstacks);
		if (!net)
		{
			Log::instance().error("Error loading net");
		}
		else
			nets.append(net);
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "nets");

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



