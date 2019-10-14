#include "CORBAmodule.h"

/***************************************************************************
	Wraps CORBA_ORB
	Methods handled:
		object_to_string
		string_to_object
		run
*/

PyTypeObject CORBA_ORB_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"CORBA.ORB",         /*tp_name*/
	sizeof(CORBA_ORB_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)CORBA_ORB_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)CORBA_ORB_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)CORBA_ORB_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	0,       /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

CORBA_ORB_PyObject *CORBA_ORB_PyObject__new(PyObject *arg)
{
	CORBA_ORB_PyObject *self;
	PyObject *orb_args;
	int argc, i;
	char *id, **argv;

	if (!PyArg_ParseTuple(arg, "Os", &orb_args, &id))
		return NULL;

	if (PyList_Check(orb_args))
		orb_args = PyList_AsTuple(orb_args);
	else if (!PyTuple_Check(orb_args)) {
		PyErr_Format(PyExc_TypeError, 
		             "parameter 1 expected either list or tuple, got %s",
		             orb_args->ob_type->tp_name);
		return NULL;
	}

	self = PyObject_NEW(CORBA_ORB_PyObject, &CORBA_ORB_PyObject_Type);
	if (!self)
		return NULL;

	CORBA_exception_init(&self->ev);

	// setup the args from orb_args
	argc = PyTuple_Size(orb_args) + 1;
	argv = g_new(char *, argc);
	argv[0] = g_strdup("orbit-python"); // kludge
	for (i = 1; i < argc; i++) {
		PyObject *o = PyObject_Repr(PyTuple_GetItem(orb_args, i - 1));
		argv[i] = PyString_AsString(o);
	}

	self->obj = CORBA_ORB_init(&argc, argv, id, &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;
	orb_instance = self;
	return self;
}

void CORBA_ORB_PyObject__dealloc(CORBA_ORB_PyObject *self)
{
	PyMem_DEL(self);
}

PyObject *CORBA_ORB_PyObject__object_to_string(CORBA_ORB_PyObject *self, 
                                               PyObject *args)
{
	CORBA_PyObject *obj;
	char *s;
	if (PyArg_ParseTuple(args, "O", &obj)) {
		// validate obj == CORBA_PyObject
		s = CORBA_ORB_object_to_string(self->obj, obj->obj, &self->ev);
		if (!check_corba_ex(&self->ev))
			return NULL;
		if (s)
			return PyString_FromString(s);
	}
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *CORBA_ORB_PyObject__string_to_object(CORBA_ORB_PyObject *self, 
                                               PyObject *args)
{
	CORBA_Object obj;
	char *s;
	if (!PyArg_ParseTuple(args, "s", &s))
		return NULL;
	obj = CORBA_ORB_string_to_object(self->obj, s, &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;
	if (obj == CORBA_OBJECT_NIL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return ((PyObject *)CORBA_Object_to_PyObject(obj));
}

PyObject *CORBA_ORB_PyObject__resolve_initial_references(
                                   CORBA_ORB_PyObject *self, PyObject *args)
{
	CORBA_Object res;
	char *s;
	if (!PyArg_ParseTuple(args, "s", &s))
		return NULL;
	// FIXME: Handle RootPOA and POACurrent
	res = CORBA_ORB_resolve_initial_references(self->obj, s, &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;
	if (!strcmp(s, "RootPOA"))
		return (PyObject *)POA_Object_to_PyObject((PortableServer_POA)res);
	return ((PyObject *)CORBA_Object_to_PyObject(res));
}

PyObject *CORBA_ORB_PyObject__run(CORBA_ORB_PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	CORBA_ORB_run(self->obj, &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}

PyMethodDef CORBA_ORB_PyObject_methods[] = {
	{ "object_to_string", (PyCFunction)CORBA_ORB_PyObject__object_to_string, 1 },
	{ "string_to_object", (PyCFunction)CORBA_ORB_PyObject__string_to_object, 1 },
	{ "resolve_initial_references", 
	  (PyCFunction)CORBA_ORB_PyObject__resolve_initial_references, 1 },
	{ "run", (PyCFunction)CORBA_ORB_PyObject__run, 1},
	{ NULL, NULL }
};

PyObject *CORBA_ORB_PyObject__getattr(CORBA_ORB_PyObject *self, char *name)
{
	self->last = g_strdup(name);
	return Py_FindMethod(CORBA_ORB_PyObject_methods, (PyObject *)self, name);
}

int CORBA_ORB_PyObject__setattr(CORBA_ORB_PyObject *self, char *n, PyObject *v)
{
	return -1;
}

