typedef struct {
   PyObject_HEAD
   CORBA_Object obj;
} CORBA_fixed_PyObject;

CORBA_fixed_PyObject *CORBA_fixed_PyObject__new(PyObject *);
void CORBA_fixed_PyObject__dealloc(CORBA_fixed_PyObject *);
PyObject *CORBA_fixed_PyObject__getattr(CORBA_fixed_PyObject *, char *);
int CORBA_fixed_PyObject__setattr(CORBA_fixed_PyObject *, char *, PyObject *);
