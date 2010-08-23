#include "PCBDoc.h"
#include "Log.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#include "Polygon.h"
#include "Area.h"

PCBDoc::PCBDoc()
{
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


//////// XML PARSING /////////
// parser methods
void loadProps(QXmlStreamReader &reader, QString &name, UNIT &units, int &defaultps);
void loadPadstacks(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks);
void loadFootprints(QXmlStreamReader &reader, QList<Footprint*> &footprints, const QHash<int, Padstack*> &padstacks);
void loadOutline(QXmlStreamReader &reader, Polygon *& poly);
void loadParts(QXmlStreamReader &reader, QList<Part*> parts, PCBDoc* doc);
void loadNets(QXmlStreamReader &reader, QList<Net*> nets, PCBDoc* doc, const QHash<int, Padstack*> &padstacks);
void loadAreas(QXmlStreamReader &reader, QList<Area*> areas, PCBDoc *doc);
void loadTexts(QXmlStreamReader &reader, QList<Text> texts);


bool validateFile(QIODevice &file)
{
	const QString fileName = ":/xpcbschema.xsd";
	QFile schemaFile(fileName);
	if (!schemaFile.open(QIODevice::ReadOnly))
	{
		Log::instance().error(QString("Error loading XML schema: ").arg(schemaFile.errorString()));
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

bool PCBDoc::loadFromFile(const QString &file)
{
	QFile inFile(file);
	if (!inFile.open(QIODevice::ReadOnly))
	{
		Log::instance().error(QString("Unable to read file: %1").arg(inFile.errorString()));
		inFile.close();
		return false;
	}
	if (!validateFile(inFile))
	{
		Log::instance().error("Unable to load file: XML validation failed");
		inFile.close();
		return false;
	}

	inFile.reset();
	QXmlStreamReader reader(&inFile);

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
			mTraceList->loadFromXml(reader);
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

void loadProps(QXmlStreamReader &reader, QString &name, UNIT &units, int &defaultps)
{
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
}

void loadPadstacks(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks)
{
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
}

void loadFootprints(QXmlStreamReader &reader, QList<Footprint*> &footprints, const QHash<int, Padstack*> &padstacks)
{
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
}

void loadOutline(QXmlStreamReader &reader, Polygon *& poly)
{
	reader.readNextStartElement();
	poly = Polygon::newFromXML(reader);
	if (!poly)
	{
		Log::instance().error("Error loading board outline");
	}
}

void loadParts(QXmlStreamReader &reader, QList<Part*> parts, PCBDoc *doc)
{
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
}

void loadNets(QXmlStreamReader &reader, QList<Net*> nets, PCBDoc *doc, const QHash<int, Padstack*> &padstacks)
{
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
}


void loadAreas(QXmlStreamReader &reader, QList<Area*> areas, PCBDoc *doc)
{
	while(reader.readNextStartElement())
	{
		Area* area = Area::newFromXML(reader, *doc);
		if (!area)
		{
			Log::instance().error("Error loading area");
		}
		else
			areas.append(area);
	}
}

void loadTexts(QXmlStreamReader &reader, QList<Text> texts)
{
	while(reader.readNextStartElement())
	{
		texts.append(Text::newFromXML(reader));
	}
}




