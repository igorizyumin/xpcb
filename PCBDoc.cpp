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
	: mUnits(XPcb::MM)
{
	mUndoStack = new QUndoStack(this);
	connect(mUndoStack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
	connect(mUndoStack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
	connect(mUndoStack, SIGNAL(cleanChanged(bool)), this, SIGNAL(cleanChanged(bool)));
}

Document::~Document()
{
	mUndoStack->clear();
	delete mUndoStack;
}

bool Document::isModified()
{
	return !mUndoStack->isClean();
}

void Document::doCommand(QUndoCommand *cmd)
{
	mUndoStack->push(cmd);
	emit changed();
}

void Document::undo()
{
	mUndoStack->undo();
	emit changed();
}

void Document::redo()
{
	mUndoStack->redo();
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
	: Document()
{
	mFp = new Footprint();
}

FPDoc::~FPDoc()
{
	clearFP();
}

void FPDoc::clearFP()
{
	delete mFp;
	mFp = NULL;
	foreach(Padstack* p, mPadstacks)
	{
		delete p;
	}
	mPadstacks.clear();
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

QList<PCBObject*> FPDoc::findObjs(QPoint &pt)
{
	QList<PCBObject*> out;

	foreach(Pin* p, mFp->pins())
	{
		if (p->bbox().contains(pt))
			out.append(p);
	}
	foreach(Text* p, mFp->texts())
	{
		if (p->bbox().contains(pt))
			out.append(p);
	}
	foreach(const Line& p, mFp->getLines())
	{
		if (p.bbox().contains(pt))
			out.append(const_cast<Line*>(&p));
	}

	if (mFp->getRefText().bbox().contains(pt))
		out.append(const_cast<Text*>(&mFp->getRefText()));
	if (mFp->getValueText().bbox().contains(pt))
		out.append(const_cast<Text*>(&mFp->getValueText()));

	return out;
}

QList<PCBObject*> FPDoc::findObjs(QRect &rect)
{
	Q_ASSERT(mFp);
	QList<PCBObject*> out;

	QList<Pin*> pins = mFp->pins();
	QList<Line> lines = mFp->getLines();
	QList<Text*> texts = mFp->texts();

	foreach(Pin* p, pins)
	{
		if (p->bbox().intersects(rect))
			out.append(p);
	}
	foreach(Text* p, texts)
	{
		if (p->bbox().intersects(rect))
			out.append(p);
	}
	foreach(const Line& p, lines)
	{
		if (p.bbox().intersects(rect))
			out.append(const_cast<Line*>(&p));
	}
	if (mFp->getRefText().bbox().intersects(rect))
		out.append(const_cast<Text*>(&mFp->getRefText()));
	if (mFp->getValueText().bbox().intersects(rect))
		out.append(const_cast<Text*>(&mFp->getValueText()));

	return out;
}

QList<Padstack*> FPDoc::padstacks()
{
	return mPadstacks;
}

void FPDoc::addPadstack(Padstack* ps)
{
	mPadstacks.append(ps);
}

void FPDoc::removePadstack(Padstack *ps)
{
	// TODO !
}

void FPDoc::addPin(Pin *p)
{
	mFp->addPin(p);
}

void FPDoc::removePin(Pin *p)
{
	mFp->removePin(p);
}

////////////////////////////// PCBDOC /////////////////////////////////

PCBDoc::PCBDoc()
		: Document(), mNumLayers(2), mTraceList(NULL), mBoardOutline(NULL), mDefaultPadstack(NULL)
{
	mTraceList = new TraceList();
	mDefaultPadstack = new Padstack();
	mPadstacks.append(mDefaultPadstack);
}

PCBDoc::~PCBDoc()
{
	clearDoc();
}

Footprint* PCBDoc::getFootprint(const QString &name)
{
	foreach(Footprint* fp, mFootprints)
	{
		if (fp->name() == name)
			return fp;
	}
	return NULL;
}

Part* PCBDoc::getPart(const QString &refdes)
{
	foreach(Part* p, mParts)
	{
		if (p->refdes() == refdes)
			return p;
	}
	return NULL;
}

Net* PCBDoc::getNet(const QString &name) const
{
	foreach(Net* p, mNets)
	{
		if (p->name() == name)
			return p;
	}
	return NULL;
}

void PCBDoc::addText(Text *t)
{
	if (!mTexts.contains(t))
		mTexts.append(t);
	else
		Log::instance().error("Text object already in document");
}

void PCBDoc::removeText(Text *t)
{
	if (mTexts.contains(t))
	{
		mTexts.removeOne(t);
		Log::instance().message("Text object deleted");
	}
	else
		Log::instance().error("Text object does not exist in document");
}

QList<PCBObject*> PCBDoc::findObjs(QRect &rect)
{
	QList<PCBObject*> out;

	foreach(Segment* s, mTraceList->segments())
	{
		if (rect.intersects(s->bbox()))
			out.append(s);
	}

	foreach(Part* p, mParts)
	{
		if (rect.intersects(p->bbox()))
			out.append(p);
		if (p->refVisible() && p->refdesText()->bbox().intersects(rect))
			out.append(p->refdesText());
		if (p->valueVisible() && p->valueText()->bbox().intersects(rect))
			out.append(p->valueText());
	}

	foreach(Text* t, mTexts)
	{
		if (rect.intersects(t->bbox()))
			out.append(t);
	}

	foreach(Area* a, mAreas)
	{
		if (rect.intersects(a->bbox()))
			out.append(a);
	}

	return out;
}

QList<PCBObject*> PCBDoc::findObjs(QPoint &pt)
{
	QList<PCBObject*> out;
	foreach(Part* p, mParts)
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
	foreach(Text* t, mTexts)
	{
		if(t->bbox().contains(pt))
			out.append(t);
	}
	return out;
}

void PCBDoc::clearDoc()
{
	delete mTraceList;
	mTraceList = NULL;

	foreach(Net* n, mNets)
		delete n;
	mNets.clear();

	foreach(Part* n, mParts)
		delete n;
	mParts.clear();

	foreach(Text* t, mTexts)
		delete t;
	mTexts.clear();

	foreach(Area* n, mAreas)
		delete n;
	mAreas.clear();

	foreach(Footprint* n, mFootprints)
		delete n;
	mFootprints.clear();

	foreach(Padstack* n, mPadstacks)
		delete n;
	mPadstacks.clear();

	mBoardOutline = Polygon();

	mDefaultPadstack = NULL;
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

void PCBDoc::addPadstack(Padstack* ps)
{
	mPadstacks.append(ps);
}

void PCBDoc::removePadstack(Padstack *ps)
{

}

//////// XML PARSING /////////
// parser methods
void loadProps(QXmlStreamReader &reader, QString &name, XPcb::UNIT &units, int &defaultps, int &numLayers);
void loadPadstacks(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks);
void loadFootprints(QXmlStreamReader &reader, QList<Footprint*> &footprints, const QHash<int, Padstack*> &padstacks);
void loadOutline(QXmlStreamReader &reader, Polygon & poly);
void loadParts(QXmlStreamReader &reader, QList<Part*>& parts, PCBDoc* doc);
void loadNets(QXmlStreamReader &reader, QList<Net*>& nets, PCBDoc* doc, const QHash<int, Padstack*> &padstacks);
void loadAreas(QXmlStreamReader &reader, QList<Area*>& areas, PCBDoc *doc);
void loadTexts(QXmlStreamReader &reader, QList<Text*>& texts);


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
	foreach(Padstack* ps, mPadstacks)
		ps->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("footprints");
	foreach(Footprint* fp, mFootprints)
		fp->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("outline");
	this->mBoardOutline.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("parts");
	foreach(Part* p, mParts)
		p->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("nets");
	foreach(Net* n, mNets)
		n->toXML(writer);
	writer.writeEndElement();

	mTraceList->toXML(writer);

	writer.writeStartElement("areas");
	foreach(Area* a, mAreas)
		a->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("texts");
	foreach(Text* t, mTexts)
		t->toXML(writer);
	writer.writeEndElement();

	writer.writeEndElement();
	writer.writeEndDocument();

	mUndoStack->setClean();
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
	mTraceList = new TraceList();

	// default padstack id
	int defaultPS;

	QHash<int, Padstack*> padstacks;

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
				mDefaultPadstack = new Padstack();
				mPadstacks.append(mDefaultPadstack);

			}
		}
		else if (t == "footprints")
			loadFootprints(reader, this->mFootprints, padstacks);
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
	clearFP();
	QHash<int, Padstack*> padstacks;

	reader.readNextStartElement();
	if (reader.name() != "xpcbFootprint")
	{
		Log::instance().error("Not a footprint.");
		return false;
	}
	while(reader.readNextStartElement())
	{
		QStringRef t = reader.name();

		if (t == "padstacks")
		{
			loadPadstacks(reader, padstacks);
			mPadstacks.append(padstacks.values());
		}
		else if (t == "footprint")
		{
			mFp = Footprint::newFromXML(reader, padstacks);
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

	writer.writeStartElement("padstacks");
	foreach(Padstack* ps, mPadstacks)
		ps->toXML(writer);
	writer.writeEndElement();

	mFp->toXML(writer);

	writer.writeEndElement();
	writer.writeEndDocument();

	mUndoStack->setClean();
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

void loadPadstacks(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks)
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
			Padstack* ps = Padstack::newFromXML(reader);
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

void loadFootprints(QXmlStreamReader &reader, QList<Footprint*> &footprints, const QHash<int, Padstack*> &padstacks)
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
			Footprint* fp = Footprint::newFromXML(reader, padstacks);
			if (!fp)
			{
				Log::instance().error("Error loading footprint");
			}
			else
				footprints.append(fp);
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

void loadParts(QXmlStreamReader &reader, QList<Part*>& parts, PCBDoc *doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "parts");
	while(reader.readNextStartElement())
	{
		Part* part = Part::newFromXML(reader, doc);
		if (!part)
		{
			Log::instance().error("Error loading part");
		}
		else
			parts.append(part);
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "parts");

}

void loadNets(QXmlStreamReader &reader, QList<Net*> &nets, PCBDoc *doc, const QHash<int, Padstack*> &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "nets");
	while(reader.readNextStartElement())
	{
		Net* net = Net::newFromXML(reader, doc, padstacks);
		if (!net)
		{
			Log::instance().error("Error loading net");
		}
		else
			nets.append(net);
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "nets");

}


void loadAreas(QXmlStreamReader &reader, QList<Area*> &areas, PCBDoc *doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "areas");
	while(reader.readNextStartElement())
	{
		Area* area = Area::newFromXML(reader, *doc);
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

void loadTexts(QXmlStreamReader &reader, QList<Text*> &texts)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "texts");
	while(reader.readNextStartElement())
	{
		texts.append(Text::newFromXML(reader));
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "texts");

}



