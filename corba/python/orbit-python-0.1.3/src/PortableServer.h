
typedef struct {
	PyObject_HEAD
	PortableServer_POAManager obj;
	CORBA_Environment ev;
} POAManager_PyObject;

extern PyTypeObject POAManager_PyObject_Type;
extern PyTypeObject POA_PyObject_Type;

PyObject *POAManager_PyObject__getattr(POAManager_PyObject *, char *);
void POAManager_PyObject__dealloc(POAManager_PyObject *);
int POAManager_PyObject__setattr(POAManager_PyObject *, char *, PyObject *);


typedef struct {
	PyObject_HEAD
	PortableServer_POA obj;
	CORBA_Environment ev;
} POA_PyObject;

void POA_PyObject__dealloc(POA_PyObject *);
PyObject *POA_PyObject__getattr(POA_PyObject *, char *);
int POA_PyObject__setattr(POA_PyObject *, char *, PyObject *);
POA_PyObject *POA_Object_to_PyObject(PortableServer_POA);

extern PyMethodDef PortableServer_methods[];

