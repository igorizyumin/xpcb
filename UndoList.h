#pragma once

#include <QList>

class UndoItem
{
public:
	virtual void undo() = 0;
	virtual void redo() = 0;
};

class UndoItemGroup : public UndoItem
{
public:
	UndoItemGroup() : m_undone(false), m_group(NULL) {}
	~UndoItemGroup();

	virtual void undo();
	virtual void redo();

	void addItem(UndoItem* item) {m_items.append(item);}
protected:
	QList<UndoItem*> m_items;
	bool m_undone;
};

class UndoList
{
public:
	CUndoList( int max_items = -1 );
	~CUndoList();

	void addItem(UndoItem* item);
	void startGroup();
	void finishGroup();

	void undo();
	void redo();

	bool isUndoPossible();
	bool isRedoPossible();

private:
	void removeExtraItems();
	void clearRedo();
	int m_max_items;
	QList<UndoItem*> m_undoStack;
	QList<UndoItem*> m_redoStack;
	UndoItemGroup * m_group;
};
