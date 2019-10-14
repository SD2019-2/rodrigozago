#include "CORBAmodule.h"

/* Generic CORBA.Object */
static PyTypeObject CORBA_fixed_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"CORBA.fixed",         /*tp_name*/
	sizeof(CORBA_fixed_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)CORBA_fixed_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)CORBA_fixed_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)CORBA_fixed_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	0,       /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

CORBA_fixed_PyObject *CORBA_fixed_PyObject__new(PyObject *arg)
{
	CORBA_fixed_PyObject *self;
	self = PyObject_NEW(CORBA_fixed_PyObject, &CORBA_fixed_PyObject_Type);
	if (!self)
		return NULL;
	return self;
}


void CORBA_fixed_PyObject__dealloc(CORBA_fixed_PyObject *self)
{
	g_message("deallocing CORBA.fixed pyobject");
	PyMem_DEL(self);
}

PyObject *CORBA_fixed_PyObject__getattr(CORBA_fixed_PyObject *self, char *name)
{
	return NULL;
}

int CORBA_fixed_PyObject__setattr(CORBA_fixed_PyObject *self, char *
                                  name, PyObject *v)
{
	return -1;
}


