%Module xpcb 0

%Import QtGui/QtGuimod.sip

%Include global.sip
%Include sharedptr.sip
%Include pcbobject.sip
%Include footprint.sip
%Include net.sip
%Include doc.sip
%Include part.sip
%Include trace.sip



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

	void selectObj(QSharedPointer<PCBObject> obj);
	void hideObj(QSharedPointer<PCBObject> obj);
	void unhideObj(QSharedPointer<PCBObject> obj);

	virtual Document* doc() = 0;
	PCBView* view();

	QPoint snapToPlaceGrid(const QPoint &p) const;
	QPoint snapToRouteGrid(const QPoint &p) const;

	bool isLayerVisible(const Layer& l) const;
	const Layer& activeLayer() const;
	
	void registerAction(CtrlAction* action);
	void installEditor(QSharedPointer<AbstractEditor> editor);

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
	
	virtual Document* doc() = 0;
	virtual Controller* ctrl() = 0;

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

class AbstractEditor : QObject
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

class AbstractSelFPDialog : QDialog
{
%TypeHeaderCode
#include "EditPartDialog.h"
%End
public:
	AbstractSelFPDialog(QWidget* parent = NULL);
	virtual bool fpSelected() = 0;
	virtual QUuid uuid() = 0;
};

class AbstractSelFPDialogFactory
{
%TypeHeaderCode
#include "EditPartDialog.h"
%End

public:
	virtual AbstractSelFPDialog* makeDialog(QWidget* parent) = 0 /Factory/;
	static AbstractSelFPDialogFactory* instance();
protected:
	void registerInstance(AbstractSelFPDialogFactory* f);
};
