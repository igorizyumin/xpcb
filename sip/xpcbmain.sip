%Module xpcb 0

%Import QtGui/QtGuimod.sip

%Include global.sip
%Include pcbobject.sip

class Text : PCBObject 
{
%TypeHeaderCode
#include "Text.h"
%End
public:
	Text();
	Text(const QPoint &pos, int angle,
		bool mirror, bool negative, const Layer& layer, int font_size,
		int stroke_width, const QString &text );

	void setParent(PCBObject *parent);

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v);
	virtual bool testHit(QPoint pt, const Layer& l) const;
//	virtual QSharedPointer<PCBObjState> getState() const;
//	virtual bool loadState(QSharedPointer<PCBObjState> &state);

	static Text* newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	QPoint pos() const;
	void setPos(const QPoint &newpos);

	const QString & text() const;
	void setText(const QString &text);

	int angle() const;
	void setAngle(int angle);

	int fontSize() const;
	void setFontSize(int size);

	int strokeWidth() const;
	void setStrokeWidth(int w);

	bool isMirrored() const;
	void setMirrored(bool b);

	bool isNegative() const;
	void setNegative(bool b);

	const Layer& layer() const;
	void setLayer(const Layer& l);


	virtual void parentChanged();

protected:
	void changed();
	void rebuild() const;

};


class Document : QObject /Abstract/
{
%TypeHeaderCode
#include "PCBDoc.h"
%End
public:
	enum LayerOrder
	{
		ListOrder,
		DrawPriorityOrder
	};
	Document();
	virtual ~Document();

	virtual bool saveToFile(const QString & file);

	virtual bool saveToXml(QXmlStreamWriter &writer) = 0;

	virtual bool loadFromFile(const QString & file);

	virtual bool loadFromFile(QFile & file) = 0;

	virtual bool loadFromXml(QXmlStreamReader &reader) = 0;

	virtual bool isModified();

	virtual void doCommand(QUndoCommand *cmd);

	virtual QList<Layer> layerList(LayerOrder order = ListOrder) = 0;

	virtual QString name() const;
	virtual XPcb::UNIT units() const;


	virtual QList<PCBObject*> findObjs(QPoint &pt) = 0;

	virtual QList<PCBObject*> findObjs(QRect &rect) = 0;


	virtual void addText(Text* t) = 0;

	virtual void removeText(Text* t) = 0;



//	virtual QList<Padstack*> padstacks() = 0;

//	virtual void addPadstack(Padstack* ps) = 0;

//	virtual void removePadstack(Padstack* ps) = 0;
signals:
	void changed();
	void cleanChanged(bool clean);
	void canUndoChanged(bool e);
	void canRedoChanged(bool e);

public slots:
	virtual void undo();
	virtual void redo();
};

class CtrlAction : QObject
{
%TypeHeaderCode
#include "Controller.h"
%End
public:
	CtrlAction(int key, QString text);

	int key() const;
	QString text() const;
	void setText(QString text);

public slots:
	void exec();

signals:
	void execFired();
};

class Controller : QObject /Abstract/
{
%TypeHeaderCode
#include "Controller.h"
%End
public:

	enum SelectionMaskT { SM_PARTS, SM_REFDES, SM_VALUE, SM_PINS, SM_TRACES,
						  SM_VERTICES, SM_AREAS, SM_TEXT, SM_CUTOUTS, SM_OUTLINE, SM_DRC };

	explicit Controller(QObject *parent = 0);

	void registerView(PCBView* view);
//	void registerActionBar(ActionBar* bar);
//	void registerLayerWidget(LayerWidget* widget);

	void draw(QPainter* painter, QRect &rect, const Layer &layer);

	bool docIsOpen();

	void selectObj(PCBObject* obj);
	void hideObj(PCBObject* obj);
	void unhideObj(PCBObject* obj);

	virtual Document* doc() = 0;
	PCBView* view();

	QPoint snapToPlaceGrid(const QPoint &p) const;
	QPoint snapToRouteGrid(const QPoint &p) const;

	bool isLayerVisible(const Layer& l) const;
	const Layer& activeLayer() const;
	
	void registerAction(CtrlAction* action);
	void installEditor(AbstractEditor* editor);

public slots:
	void onPlaceGridChanged(int grid);
	void onRouteGridChanged(int grid);

protected slots:
	void onEditorOverlayChanged();
	void onEditorFinished();
	void onEditorActionsChanged();
	void onDocumentChanged();

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void updateEditor();
	void updateActions();
};

class PCBView : QWidget
{
%TypeHeaderCode
#include "PCBView.h"
%End
public:
        PCBView(QWidget *parent);
        ~PCBView();


	virtual QSize sizeHint() const;

	void setCtrl(Controller* ctrl);

	const QTransform& transform() const;

signals:
	void mouseMoved(QPoint pt);

public slots:
	void visGridChanged(int grid);

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);
	virtual void wheelEvent(QWheelEvent *);
};

class MainWindow : QMainWindow /Abstract/
{
%TypeHeaderCode
#include "mainwindow.h"
%End

public:
	MainWindow(QWidget *parent /TransferThis/ = 0);

protected slots:
	void on_actionAbout_triggered();
	virtual void onViewCoords(QPoint pt);
	void documentWasModified();
	virtual void on_actionNew_triggered();
	virtual void on_actionOpen_triggered();
	virtual bool on_actionSave_triggered();
	virtual bool on_actionSave_as_triggered();
	virtual bool on_actionClose_triggered();
	void on_action_Undo_triggered();
	void on_action_Redo_triggered();
	void onUndoAvailableChanged(bool enabled);
	void onRedoAvailableChanged(bool enabled);

protected:
	virtual void closeEvent(QCloseEvent *event);
	virtual bool maybeSave();
	virtual void newDoc() = 0;
	virtual void closeDoc() = 0;
	virtual bool loadFile(const QString &fileName);
	virtual bool saveFile(const QString &fileName);
	virtual void setCurrentFile(const QString &fileName);
	QString strippedName(const QString &fullFileName);

	virtual Document* doc() = 0;
	virtual Controller* ctrl() = 0;

	virtual void loadGeom() = 0;
	virtual void saveGeom() = 0;
};

class FPEditWindow : MainWindow
{
%TypeHeaderCode
#include "mainwindow.h"
%End
public:
	FPEditWindow(QWidget *parent /TransferThis/ = 0, QList<Plugin*> plugins = QList<Plugin*>());

protected:
	virtual void newDoc();
	virtual void closeDoc();
	virtual Document* doc();
	virtual Controller* ctrl();
	virtual void loadGeom();
	virtual void saveGeom();
};

class PCBEditWindow : MainWindow
{
%TypeHeaderCode
#include "mainwindow.h"
%End

public:
	PCBEditWindow(QWidget *parent /TransferThis/ = 0, QList<Plugin*> plugins = QList<Plugin*>());

protected:
	virtual void newDoc();
	virtual void closeDoc();
	virtual Document* doc();
	virtual Controller* ctrl();
	virtual void loadGeom();
	virtual void saveGeom();
};


class Plugin
{
%TypeHeaderCode
#include "Plugin.h"
%End
public:
    Plugin();

	virtual void installWidgets(FPEditWindow& window) = 0;
	virtual void installWidgets(PCBEditWindow& window) = 0;

};

class AbstractEditor : QObject /Abstract/
{
%TypeHeaderCode
#include "Editor.h"
%End
public:
	explicit AbstractEditor(Controller *ctrl);
	virtual void init();
	virtual void drawOverlay(QPainter* painter) = 0;
	virtual QList<const CtrlAction*> actions() const;

signals:
	void actionsChanged();
	void overlayChanged();
	void editorFinished();

public slots:

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent *event);
	
	Controller* ctrl();
};

