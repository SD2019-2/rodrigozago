typedef struct {
   PyObject_HEAD
	PyObject *value, *tc_object;
} CORBA_any_PyObject;

extern PyTypeObject CORBA_any_PyObject_Type;

#define CORBA_any_PyObject_Check(v) \
		((v)->ob_type == &CORBA_any_PyObject_Type)

CORBA_any_PyObject *CORBA_any_PyObject__new(PyObject *);
void CORBA_any_PyObject__dealloc(CORBA_any_PyObject *);
PyObject *CORBA_any_PyObject__getattr(CORBA_any_PyObject *, char *);
int CORBA_any_PyObject__setattr(CORBA_any_PyObject *, char *, PyObject *);
PyObject *CORBA_any_PyObject__repr(CORBA_any_PyObject *);
