#include "PCBDoc.h"
#include "Log.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#include "Polygon.h"
#include "Area.h"
#include "Trace.h"

PCBDoc::PCBDoc()
		: QObject(), mModified(false), mUnits(MM), mTraceList(NULL), mBoardOutline(NULL)
{
	mTraceList = new TraceList();
}

PCBDoc::~PCBDoc()
{
	delete mTraceList;
	delete mBoardOutline;
	foreach(Net* n, mNets)
	{
		delete n;
	}
	foreach(Part* n, mParts)
	{
		delete n;
	}
	foreach(Area* n, mAreas)
	{
		delete n;
	}
	foreach(Footprint* n, mFootprints)
	{
		delete n;
	}
	foreach(Padstack* n, mPadstacks)
	{
		delete n;
	}

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

void PCBDoc::draw(QPainter *painter, QRect rect, PCBLAYER layer)
{
	foreach(Segment* s, mTraceList->segments())
	{
		if (rect.intersects(s->bbox()))
			s->draw(painter, layer);
	}

	foreach(Part* p, mParts)
	{
		if (rect.intersects(p->bbox()))
			p->draw(painter, layer);
	}

	foreach(const Text& t, mTexts)
	{
	//	if (rect.intersects(t.bbox()))
			t.draw(painter, layer);
	}

	foreach(Area* a, mAreas)
	{
		if (rect.intersects(a->bbox()))
			a->draw(painter, layer);
	}

}

//////// XML PARSING /////////
// parser methods
void loadProps(QXmlStreamReader &reader, QString &name, UNIT &units, int &defaultps);
void loadPadstacks(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks);
void loadFootprints(QXmlStreamReader &reader, QList<Footprint*> &footprints, const QHash<int, Padstack*> &padstacks);
void loadOutline(QXmlStreamReader &reader, Polygon *& poly);
void loadParts(QXmlStreamReader &reader, QList<Part*>& parts, PCBDoc* doc);
void loadNets(QXmlStreamReader &reader, QList<Net*>& nets, PCBDoc* doc, const QHash<int, Padstack*> &padstacks);
void loadAreas(QXmlStreamReader &reader, QList<Area*>& areas, PCBDoc *doc);
void loadTexts(QXmlStreamReader &reader, QList<Text>& texts);


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

bool PCBDoc::loadFromFile(const QString& path)
{
	QFile inFile(path);
	if (!inFile.open(QIODevice::ReadOnly))
	{
		Log::instance().error(QString("Unable to open file %1").arg(path));
		return false;
	}
	bool ret = loadFromFile(inFile);
	inFile.close();
	return ret;
}

bool PCBDoc::saveToFile(const QString &file)
{
	Log::instance().error(QString("Not implemented"));
	return false;
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
	// default padstack id
	int defaultPS;

	QHash<int, Padstack*> padstacks;

	while(reader.readNextStartElement())
	{
		QStringRef t = reader.name();
		if (t == "props")
			loadProps(reader, mName, mUnits, defaultPS );
		else if (t == "padstacks")
			loadPadstacks(reader, padstacks);
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
		mModified = false;
	return true;

}

void loadProps(QXmlStreamReader &reader, QString &name, UNIT &units, int &defaultps)
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
				units = MM;
			else if (unitsStr == "mils")
				units = MIL;
		}
		else if (t == "defaultPadstack")
		{
			defaultps = reader.readElementText().toInt();
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

void loadOutline(QXmlStreamReader &reader, Polygon *& poly)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "outline");
	reader.readNextStartElement();
	poly = Polygon::newFromXML(reader);
	if (!poly)
	{
		Log::instance().error("Error loading board outline");
	}
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

void loadTexts(QXmlStreamReader &reader, QList<Text> &texts)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "texts");
	while(reader.readNextStartElement())
	{
		texts.append(Text::newFromXML(reader));
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "texts");

}




