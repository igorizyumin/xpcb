#include "Log.h"

class PySharedPtr
{
	public:
		PySharedPtr(PyObject* pyObj, void* cppObj, sipTypeDef* type, void (*incFunc)(void*), void (*delFunc)(void*)) 
			: mObj(pyObj), mCppObj(cppObj), mType(type), mIncFunc(incFunc), mDelFunc(delFunc) { Log::instance().message("PySharedPtr constructed"); }
		PySharedPtr(const PySharedPtr& other)
			: mObj(other.mObj), mCppObj(other.mCppObj), mType(other.mType), mIncFunc(other.mIncFunc), mDelFunc(other.mDelFunc)
		{
			Log::instance().message("PySharedPtr copied"); 
			Py_INCREF(mObj);
			(*mIncFunc)(mCppObj);
		}
		~PySharedPtr() 
		{ 
			Log::instance().message("PySharedPtr destroyed");
			(*mDelFunc)(mCppObj); 
			Py_DECREF(mObj);
		}

		PyObject* mObj;
		void* mCppObj;
		sipTypeDef* mType;
		void (*mIncFunc)(void*);
		void (*mDelFunc)(void*);
};
