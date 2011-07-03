
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

template <class T>
class sharedPtrManager {
public:
	static void addRef(QSharedPointer<T> & ptr)
	{
		Log::instance().message("AddRef called");
		if(ptrMap.contains(ptr.data()))
			ptrMap[ptr.data()].append(ptr);
		else
		{
			QList<QSharedPointer<T> > l;
			l << ptr;
			ptrMap.insert(ptr.data(), l);
		}
	}

	static void incRef(void* tv)
	{
		Log::instance().message("incRef called");
		T* t = reinterpret_cast<T*>(tv);
		if (ptrMap.contains(t))
			ptrMap[t].append(ptrMap.value(t)[0]);
	}

	static void decRef(void* tv)
	{
		Log::instance().message("decRef called");
		T* t = reinterpret_cast<T*>(tv);
		if (ptrMap.contains(t))
		{
			ptrMap[t].removeLast();
			if (ptrMap[t].isEmpty())
				ptrMap.remove(t);
		}
	}
	
	static bool hasRef(void* tv)
	{
		T* t = reinterpret_cast<T*>(tv);
		return ptrMap.contains(t);
	}

	static QHash<T*, QList<QSharedPointer<T> > > ptrMap;
};

template<class T> QHash<T*, QList<QSharedPointer<T> > > sharedPtrManager<T>::ptrMap;

%End

%ModuleCode
//QHash<Footprint*, QList<QSharedPointer<Footprint> > > sharedPtrManager<Footprint>::ptrMap;
%End

template<TYPE>
%MappedType QSharedPointer<TYPE>
{
%TypeHeaderCode
#include "PySharedPtr.h"

%End

%ConvertToTypeCode
	// from python object
	if (sipIsErr == NULL)
	{
		// check that the object is a pointer and that it is of the right type
		if (!PyInstance_Check(sipPy))
			return 0;
		if (!sipCanConvertToType(sipPy, sipType_PySharedPtr, 0))
			return 0;
		// XXX check if this is legit
		int state;
		PySharedPtr* p = reinterpret_cast<PySharedPtr*>(sipConvertToType(sipPy, sipType_PySharedPtr, 0, 0, &state, sipIsErr));
		if (*sipIsErr)
			return 0;
		bool isValid = (p->mType == sipType_TYPE);
		if (state)
			sipReleaseType(p, sipType_PySharedPtr, state);
		return isValid ? 1 : 0;
	}
	// get the wrapper object
	PySharedPtr* p = reinterpret_cast<PySharedPtr*>(sipConvertToType(sipPy, sipType_PySharedPtr, 0, 0, 0, sipIsErr));
	if (*sipIsErr)
		return 0;
	// check if we have this pointer
	if (!sharedPtrManager<TYPE>::hasRef(p->mCppObj)))
		return 0; // for now, this is an error; TODO allow python objs to be returned
	// make another reference and give it to C++
	QSharedPointer<TYPE> *ptr = new QSharedPointer<TYPE>(sharedPtrManager<TYPE>::ptrMap[reinterpret_cast<TYPE*>(p->mCppObj)][0]);
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

	// take a reference
	QSharedPointer<TYPE> *cPtr = reinterpret_cast<QSharedPointer<TYPE>*>(sipCpp);
	if (cPtr->isNull())
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	sharedPtrManager<TYPE>::addRef(*cPtr);
	PySharedPtr *p = new PySharedPtr(sipConvertFromType(cPtr->data(), sipType_TYPE, NULL), cPtr->data(), sipType_TYPE, &sharedPtrManager<TYPE>::incRef, &sharedPtrManager<TYPE>::decRef);
	PyObject *obj = sipConvertFromType(p, sipType_PySharedPtr, Py_None);
	return obj;
%End
};
