
class PySharedPtr
{
%TypeHeaderCode
#include "PySharedPtr.h"
%End
public:
	PySharedPtr(const PySharedPtr& other);
	~PySharedPtr();

	SIP_PYOBJECT obj();
	%MethodCode
		Py_INCREF(sipCpp->mObj);
		sipRes = sipCpp->mObj;
	%End

};

%ModuleHeaderCode
#include <QSharedPointer>
#include <QHash>
#include <QList>
#include "Shape.h"

struct PtrDescriptor
{
	PtrDescriptor() : refCnt(0), sharedPtrV(NULL), deleteFunc(NULL) {}
	PtrDescriptor(void* ptr, void (*delFunc)(void*)) 
		: refCnt(1), sharedPtrV(ptr), deleteFunc(delFunc) {}

	// reference count
	int refCnt;
	// pointer to shared pointer object
	void* sharedPtrV;
	// the destructor function for the appropriate pointer type
	void (*deleteFunc)(void*);
};


class SharedPtrMgr 
{
public:
	static bool hasRef(void* tv)
	{
		return ptrMap.contains(tv);
	}

	static void incRef(void* tv)
	{
		if (ptrMap.contains(tv))
			ptrMap[tv].refCnt++;
		Log::instance().message("incRef called");
	}
	
	static void decRef(void* tv)
	{
		Log::instance().message("decRef called");
		if (ptrMap.contains(tv))
			// delete the shared pointer when refcount hits 0
			if (--ptrMap[tv].refCnt == 0)
			{
				Log::instance().message("decRef deleting reference");
				(*ptrMap[tv].deleteFunc)(ptrMap[tv].sharedPtrV);
				ptrMap.remove(tv);
			}
	}

	static void addRef(void* tv, void* sharedPtrV, void (*deleteFunc)(void*))
	{
		if (!ptrMap.contains(tv))
			ptrMap.insert(tv, PtrDescriptor(sharedPtrV, deleteFunc));
	}

	static void* takeRef(void* tv)
	{
		if (!ptrMap.contains(tv)) return NULL;
		ptrMap[tv].refCnt++;
		return ptrMap[tv].sharedPtrV;
		Log::instance().message("takeRef called");
	}

	static bool checkType(void* tv, void (*deleteFunc)(void*))
	{
		if (ptrMap.contains(tv) && ptrMap[tv].deleteFunc == deleteFunc)
			return true;
		return false;
	}

private:
	static QHash<void*, PtrDescriptor > ptrMap;
};

template <class T>
class SharedPtrType {
public:
	static void addRef(QSharedPointer<T> & ptr)
	{
		SharedPtrMgr::addRef(reinterpret_cast<void*>(ptr.data()), reinterpret_cast<void*>(new QSharedPointer<T>(ptr)), &delFunc);
	}

	static bool checkType(void* ptr)
	{
		return SharedPtrMgr::checkType(ptr, &delFunc);
	}

	static QSharedPointer<T> takeRef(void* ptrV)
	{
		if (!checkType(ptrV)) return QSharedPointer<T>();
		return QSharedPointer<T>(*reinterpret_cast<QSharedPointer<T>* > (SharedPtrMgr::takeRef(ptrV)));
	}

private:
	static void delFunc(void* ptrV)
	{
		QSharedPointer<T>* ptr = reinterpret_cast<QSharedPointer<T>* >(ptrV);
		delete ptr;
	}
};

%End

%ModuleCode
QHash<void*, PtrDescriptor > SharedPtrMgr::ptrMap;
%End

template<TYPE>
%MappedType QSharedPointer<TYPE>
{

%ConvertToTypeCode
	// from python object
	if (sipIsErr == NULL)
	{
		// check that the object is of the right type
		if (!PyInstance_Check(sipPy))
			return 0;
		if (!sipCanConvertToType(sipPy, sipType_TYPE, 0))
			return 0;
		void* ptr = sipConvertToVoidPtr(sipPy);
		if (PyErr_Occurred())
			return 0;
		if (!SharedPtrMgr::hasRef(ptr))
			return 0; // TODO allow python objs to be returned
		if (!SharedPtrType<TYPE>::checkType(ptr))
			return 0;
	}
	// get the wrapper object
	void* p = sipConvertToType(sipPy, sipType_TYPE, 0, 0, 0, sipIsErr);
	if (*sipIsErr)
		return 0;
	// make another reference and give it to C++
	QSharedPointer<TYPE> *ptr = new QSharedPointer<TYPE>(SharedPtrType<TYPE>::takeRef(p));
	*sipCppPtr = ptr;
	return sipGetState(sipTransferObj);
%End
%ConvertFromTypeCode
	// to python object
	if (!sipCpp)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	QSharedPointer<TYPE> *cPtr = reinterpret_cast<QSharedPointer<TYPE>*>(sipCpp);
	if (cPtr->isNull())
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	SharedPtrType<TYPE>::addRef(*cPtr);
	PyObject* obj = sipConvertFromType(cPtr->data(), sipType_TYPE, Py_None);
	return obj;
%End
};


template<TYPE>
%MappedType QList<QSharedPointer<TYPE> >
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ((l = PyList_New(sipCpp->size())) == NULL)
        return NULL;

    // Set the list elements.
    for (int i = 0; i < sipCpp->size(); ++i)
    {
		QSharedPointer<TYPE> t = sipCpp->at(i);
		PyObject *tobj;
		if (t.isNull())
		{
			Py_INCREF(Py_None);
			tobj = Py_None;
		}
		else
		{
			SharedPtrType<TYPE>::addRef(t);
    	    if ((tobj = sipConvertFromType(reinterpret_cast<void*>(t.data()), sipType_TYPE, Py_None)) == NULL)
       		{
            	Py_DECREF(l);
            	return NULL;
        	}
		}
        PyList_SET_ITEM(l, i, tobj);
    }

    return l;
%End

%ConvertToTypeCode
    SIP_SSIZE_T len;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PySequence_Check(sipPy) || (len = PySequence_Size(sipPy)) < 0)
            return 0;

        for (SIP_SSIZE_T i = 0; i < len; ++i)
        {
            PyObject *itm = PySequence_ITEM(sipPy, i);
            bool ok = (itm && sipCanConvertToType(itm, sipType_TYPE, 0));
			if (ok)
			{
				void* ptr = sipConvertToVoidPtr(sipPy);
				ok = !(PyErr_Occurred() || !SharedPtrMgr::hasRef(ptr)
					|| !SharedPtrType<TYPE>::checkType(ptr));
			}

            Py_XDECREF(itm);

            if (!ok)
                return 0;
        }

        return 1;
    }

	QList<QSharedPointer<TYPE> > *ql = new QList<QSharedPointer<TYPE> >;
	len = PySequence_Size(sipPy);

    for (SIP_SSIZE_T i = 0; i < len; ++i)
    {
        PyObject *itm = PySequence_ITEM(sipPy, i);
		void* p = sipConvertToType(itm, sipType_TYPE, 0, 0, 0, sipIsErr);
		if (*sipIsErr)
			return 0;

        Py_DECREF(itm);
 
        if (*sipIsErr)
        {
            delete ql;
            return 0;
        }

		ql->append(SharedPtrType<TYPE>::takeRef(p));
    }
 
    *sipCppPtr = ql;
 
    return sipGetState(sipTransferObj);

%End
};

