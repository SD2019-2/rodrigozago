typedef struct {
	PyObject_HEAD
	CORBA_ORB obj;
	CORBA_Environment ev;
	char *last;
} CORBA_ORB_PyObject;

#define CORBA_ORB_PyObject_Check(v) ((v)->ob_type == &CORBA_ORB_PyObject_Type)

extern CORBA_ORB_PyObject *orb_instance;
extern PyTypeObject CORBA_ORB_PyObject_Type;

CORBA_ORB_PyObject *CORBA_ORB_PyObject__new(PyObject *);
void CORBA_ORB_PyObject__dealloc(CORBA_ORB_PyObject *);
int CORBA_ORB_PyObject__setattr(CORBA_ORB_PyObject *, char *, PyObject *);
PyObject *CORBA_ORB_PyObject__getattr(CORBA_ORB_PyObject *, char *);

