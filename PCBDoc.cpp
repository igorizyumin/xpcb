#include "PCBDoc.h"
#include "Log.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>

PCBDoc::PCBDoc()
{
}




//////// XML PARSING /////////
// parser methods
void loadProps(QXmlStreamReader &reader, QString &name, UNIT &units, int &defaultps);
void loadPadstacks(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks);
void loadFootprints(QXmlStreamReader &reader, QList<Footprint*> &footprints, const QHash<int, Padstack*> &padstacks);
void loadOutline(QXmlStreamReader &reader, Polygon *& poly);
void loadParts(QXmlStreamReader &reader, QList<Part*> parts);
void loadNets(QXmlStreamReader &reader, QList<Net*> nets);
void loadAreas(QXmlStreamReader &reader, QList<Area*> areas);
void loadTexts(QXmlStreamReader &reader, QList<Text*> texts);


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

bool PCBDoc::loadFile(const QString &file)
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
	QXmlStreamReader reader(inFile);

	// default padstack id
	int defaultPS;

	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::StartElement)
		{
			switch(reader.name())
			{
			case "props":
				loadProps(reader, mName, mUnits, defaultPS );
				break;
			case "padstacks":
				loadPadstacks(reader);
				break;
			case "footprints":
				loadFootprints(reader);
				break;
			case "outline":
				loadOutline(reader);
				break;
			case "parts":
				loadParts(reader);
				break;
			case "nets":
				loadNets(reader);
				break;
			case "traces":
				mTraceList = TraceList::newFromXML(reader);
				break;
			case "areas":
				loadAreas(reader);
				break;
			case "texts":
				loadTexts(reader);
				break;
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

void loadProps(QXmlStreamReader &reader, QString &name, UNIT &units, int &defaultps)
{
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of properties block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		switch(reader.name())
		{
		case "name":
			name = reader.readElementText();
			break;
		case "units":
			QString unitsStr = reader.readElementText();
			if (unitsStr == "mm")
				units = MM;
			else if (unitsStr == "mils")
				units = MIL;
			break;
		case "defaultPadstack":
			defaultps = reader.readElementText().toInt();
			break;
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
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of outline block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "polyline")
		{
			poly = Polygon::newFromXML(reader);
			if (!poly)
			{
				Log::instance().error("Error loading board outline");
			}
		}
	}
}

void loadParts(QXmlStreamReader &reader, QList<Part*> parts)
{
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of parts block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "part")
		{
			Part* part = Part::newFromXML(reader);
			if (!part)
			{
				Log::instance().error("Error loading part");
			}
			else
				parts.append(part);
		}
	}
}

void loadNets(QXmlStreamReader &reader, QList<Net*> nets)
{
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of nets block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "net")
		{
			Net* net = Net::newFromXML(reader);
			if (!net)
			{
				Log::instance().error("Error loading net");
			}
			else
				nets.append(net);
		}
	}
}


void loadAreas(QXmlStreamReader &reader, QList<Area*> areas)
{
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of areas block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "area")
		{
			Area* area = Area::newFromXML(reader);
			if (!area)
			{
				Log::instance().error("Error loading area");
			}
			else
				areas.append(part);
		}
	}
}

void loadTexts(QXmlStreamReader &reader, QList<Text*> texts)
{
	while(!reader.atEnd() && !reader.hasError())
	{
		QXmlStreamReader::TokenType tok = reader.readNext();
		if (tok == QXmlStreamReader::EndElement)
			return; // end of texts block
		if (tok != QXmlStreamReader::StartElement)
			continue; // comments, etc.
		if (reader.name() == "text")
		{
			Text* text = Text::newFromXML(reader);
			if (!text)
			{
				Log::instance().error("Error loading text");
			}
			else
				texts.append(text);
		}
	}
}




