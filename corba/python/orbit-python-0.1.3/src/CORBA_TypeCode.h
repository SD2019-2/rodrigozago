typedef struct {
   PyObject_HEAD
   CORBA_TypeCode tc;	
	char *repo_id;
} CORBA_TypeCode_PyObject;

extern PyTypeObject CORBA_TypeCode_PyObject_Type;

#define CORBA_TypeCode_PyObject_Check(v) \
		((v)->ob_type == &CORBA_TypeCode_PyObject_Type)


CORBA_TypeCode_PyObject *CORBA_TypeCode_PyObject__new(PyObject *);
void CORBA_TypeCode_PyObject__dealloc(CORBA_TypeCode_PyObject *);
PyObject *CORBA_TypeCode_PyObject__getattr(CORBA_TypeCode_PyObject *, 
                                           char *);
int CORBA_TypeCode_PyObject__setattr(CORBA_TypeCode_PyObject *, char *, 
                                     PyObject *);
PyObject *CORBA_TypeCode_PyObject__repr(CORBA_TypeCode_PyObject *);
