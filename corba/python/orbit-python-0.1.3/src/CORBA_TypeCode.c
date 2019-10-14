#include "CORBAmodule.h"

/* Generic CORBA.Object */
PyTypeObject CORBA_TypeCode_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"CORBA.TypeCode",         /*tp_name*/
	sizeof(CORBA_TypeCode_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)CORBA_TypeCode_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)CORBA_TypeCode_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)CORBA_TypeCode_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	(reprfunc)CORBA_TypeCode_PyObject__repr, /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

CORBA_TypeCode_PyObject *CORBA_TypeCode_PyObject__new(PyObject *arg)
{
	PyObject *o;
	char *repo_id;
	CORBA_TypeCode tc;
	CORBA_TypeCode_PyObject *self;

	PyArg_ParseTuple(arg, "O", &o);
	if (PyString_Check(o))
		repo_id = PyString_AsString(o);
	else {
		PyObject *repo_obj = PyObject_GetAttrString(o, "__repo_id");
		if (!repo_obj) {
			raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
			                       CORBA_COMPLETED_NO,
			                       "Object has no associated repository id");
			return NULL;
		}
		repo_id = PyString_AsString(repo_obj);
	}
	tc = find_typecode(repo_id);
	if (!tc) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_NO,
		                       "Unregistered repository id passed");
		return NULL;
	}

	self = PyObject_NEW(CORBA_TypeCode_PyObject, &CORBA_TypeCode_PyObject_Type);
	if (self) {
		self->tc = tc;
/*		self->repo_id = find_repo_id_from_typecode(tc);
		if (self->repo_id)
			self->repo_id = g_strdup(self->repo_id);
*/
		self->repo_id = g_strdup(repo_id); // This is more narrow than the above
	}
	else
		CORBA_Object_release((CORBA_Object)tc, NULL);
		
	return self;
}


void CORBA_TypeCode_PyObject__dealloc(CORBA_TypeCode_PyObject *self)
{	
	if (self->repo_id)
		g_free(self->repo_id);
	PyMem_DEL(self);
}

PyObject *CORBA_TypeCode_PyObject__getattr(CORBA_TypeCode_PyObject *self, 
                                           char *name)
{
	return NULL;
}

int CORBA_TypeCode_PyObject__setattr(CORBA_TypeCode_PyObject *self, 
                                     char *name, PyObject *v)
{
	return -1;
}

PyObject *CORBA_TypeCode_PyObject__repr(CORBA_TypeCode_PyObject *self)
{
	return PyString_FromString(self->repo_id);
}
