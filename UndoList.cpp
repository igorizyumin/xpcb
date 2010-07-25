#include "UndoList.h"

UndoItemGroup::~UndoItemGroup()
{
	for(int i = 0; i < m_items.size(); i++)
	{
		delete m_items.at(i);
	}
}

void UndoItemGroup::undo()
{
	if (m_undone) return;

	for(int i = m_items.size()-1; i>=0; i--)
	{
		m_items.at(i)->undo();
	}
	m_undone = true;
}

void UndoItemGroup::redo()
{
	if (!m_undone) return;

	for(int i = 0; i<m_items.size(); i++)
	{
		m_items.at(i)->redo();
	}
	m_undone = false;
}

UndoList::UndoList( int max_items )
{
	this->m_max_items = max_items > 0 ? max_items : 0;
}

UndoList::~UndoList()
{
	if (m_group)
		delete m_group;
	for(int i = 0; i < m_undoStack.size(); i++)
	{
		delete m_undoStack[i];
	}
	for(int i = 0; i < m_redoStack.size(); i++)
	{
		delete m_redoStack[i];
	}
}


void UndoList::addItem(UndoItem* item)
{
	clearRedo();
	if (!m_group)
		m_undoStack.append(item);
	else
		m_group->addItem(item);
}

void UndoList::startGroup()
{
	if (m_group)
		finishGroup();
	m_group = new UndoItemGroup();
}

void UndoList::finishGroup()
{
	if (m_group)
	{
		clearRedo();
		m_undoStack.append(group);
		m_group = NULL;
	}
}

void UndoList::undo()
{
	if (!isUndoPossible())
		return;
	UndoItem* item = m_undoStack.takeLast();
	item->undo();
	m_redoStack.append(item);
}

void UndoList::redo()
{
	if (!isRedoPossible())
		return;
	UndoItem* item = m_redoStack.takeLast();
	item->redo();
	m_undoStack.append(item);
}

bool UndoList::isUndoPossible()
{
	return m_undoStack.size()>0;
}

bool UndoList::isRedoPossible()
{
	return m_redoStack.size()>0;
}

void UndoList::removeExtraItems()
{
	if (!m_max_items)
		return;
	while(m_undoStack.size()>m_max_items)
	{
		UndoItem * item = m_undoStack.takeFront();
		delete item;
	}
}

void UndoList::clearRedo()
{
	while(m_redoStack.size()>0)
	{
		UndoItem* item = m_redoStack.takeLast();
		delete item;
	}
}
