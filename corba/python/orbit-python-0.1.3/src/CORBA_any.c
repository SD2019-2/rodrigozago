#include "CORBAmodule.h"

/* Generic CORBA.Object */
PyTypeObject CORBA_any_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"CORBA.Any",         /*tp_name*/
	sizeof(CORBA_any_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)CORBA_any_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)CORBA_any_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)CORBA_any_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	(reprfunc)CORBA_any_PyObject__repr, /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

CORBA_any_PyObject *CORBA_any_PyObject__new(PyObject *arg)
{
	PyObject *tc_object, *value;
	CORBA_any_PyObject *self;

	PyArg_ParseTuple(arg, "OO", &tc_object, &value);
	if (!CORBA_TypeCode_PyObject_Check(tc_object)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_NO,
		                       "Parameter 1 must be a CORBA.TypeCode");
		return NULL;
	}

	self = PyObject_NEW(CORBA_any_PyObject, &CORBA_any_PyObject_Type);
	if (self) {
		Py_INCREF(tc_object);
		Py_INCREF(value);
		self->tc_object = tc_object;
		self->value = value;
	}
	return self;
}


void CORBA_any_PyObject__dealloc(CORBA_any_PyObject *self)
{
	Py_DECREF(self->value);
	Py_DECREF(self->tc_object);
	PyMem_DEL(self);
}

PyObject *CORBA_any_PyObject__getattr(CORBA_any_PyObject *self, char *name)
{
	if (!strcmp(name, "tc"))
		return self->tc_object;
	else if (!strcmp(name, "value"))
		return self->value;
	return NULL;
}

int CORBA_any_PyObject__setattr(CORBA_any_PyObject *self, 
                                     char *name, PyObject *v)
{
	return -1;
}

PyObject *CORBA_any_PyObject__repr(CORBA_any_PyObject *self)
{
	PyObject *tc_rep_obj, *val_rep_obj, *ret;
	char *tc_repr = "Unknown",  *val_repr = "Unknown", *s;

	if ((tc_rep_obj = PyObject_Repr(self->tc_object)))
		tc_repr = PyString_AsString(tc_rep_obj);
	if ((val_rep_obj = PyObject_Repr(self->value)))
		val_repr = PyString_AsString(val_rep_obj);
	s = g_strconcat("<Type ", tc_repr, " with value ", val_repr, ">", NULL);
	Py_XDECREF(tc_rep_obj);
	Py_XDECREF(val_rep_obj);
	ret = PyString_FromString(s);
	g_free(s);
	return ret;
}
