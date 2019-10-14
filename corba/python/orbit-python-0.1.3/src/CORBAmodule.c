#include "CORBAmodule.h"

PyObject *ModuleDict, *MainDict, *poa_instances, *main_module;
GHashTable *object_glue;
CORBA_ORB_PyObject *orb_instance;

/***************************************************************************
	Wraps CORBA_Object
	Methods handled:
		generated dynamically
*/

/* Generic CORBA.Object */
static PyTypeObject CORBA_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"CORBA.Object",         /*tp_name*/
	sizeof(CORBA_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)CORBA_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)CORBA_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)CORBA_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	0,       /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

CORBA_PyObject *CORBA_Object_to_PyObject(CORBA_Object object)
{
	CORBA_PyObject *self;
	CORBA_PyObject_Glue *glue;
	PyTypeObject *type;

	if (!object) {
		Py_INCREF(Py_None);
		return (CORBA_PyObject *)Py_None;
	}
	glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue, 
	                                                  object->object_id);
	type = glue ? glue->type : &CORBA_PyObject_Type;
	// FIXME: check to see if this object exists as a PyObject before creating
	self = PyObject_NEW(CORBA_PyObject, type);
	if (!self) { 
		PyErr_Print();
		return NULL;
	}
	self->repo_id = g_strdup(object->object_id);
	self->last_method = NULL;
	CORBA_exception_init(&self->ev);
	self->obj = object;
	return self;
}

void CORBA_PyObject__dealloc(CORBA_PyObject *self)
{
	g_free(self->repo_id);
	if (self->last_method)
		g_slist_free(self->last_method);
	PyMem_DEL(self);
}

PyObject *CORBA_PyObject__getattr(CORBA_PyObject *self, char *name)
{
	CORBA_PyObject_Glue *glue;
	if (!strcmp(name, "__repo_id"))
		return PyString_FromString(self->repo_id);

	glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue,self->repo_id);
	if (glue) {
		CORBA_AttributeDescription *ad;
		if (find_operation(glue, name)) {
		// push this name onto the last_method stack
			PyObject *o;
			self->last_method = g_slist_prepend(self->last_method, g_strdup(name));
			o = PyCFunction_New(glue->generic_method, (PyObject *)self);
			return o;
		}
		// Check for the existance of this attribute.
		ad = find_attribute(glue, name);
		if (ad)
			return get_attribute(self, ad);
	}

	// Couldn't find it at this point.  Raise an exception
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

int CORBA_PyObject__setattr(CORBA_PyObject *self, char *name, PyObject *v)
{
	CORBA_PyObject_Glue *glue;
	glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue,self->repo_id);

	if (glue) {
		CORBA_AttributeDescription *ad = find_attribute(glue, name);
		if (ad)
			return set_attribute(self, ad, v);
	}

	// Return with an error this attribute isn't defined
	// XXX: maybe we should let users define local attributes?
	PyErr_Format(ErrorObject, "attribute %s unknown", name);
	return -1;
}



/***************************************************************************
	main CORBA module
	Methods handled:
		ORB_init
		Object_cast
*/

PyObject *CORBA__ORB_init(PyObject *self, PyObject *args)
{
	return (PyObject *)CORBA_ORB_PyObject__new(args);
}

PyObject *CORBA__Object_cast(PyObject *self, PyObject *args)
{
	PyObject *object;
	if (!PyArg_ParseTuple(args, "O", &object))
		return NULL;
	return (PyObject *) 
		CORBA_Object_to_PyObject( ((CORBA_PyObject *)object)->obj);
}

PyObject *CORBA__load_idl(PyObject *self, PyObject *args)
{
	char *file, *params = NULL;
	if (PyArg_ParseTuple(args, "s|s", &file, &params))
		if (parse(file, params)) {
			Py_INCREF(Py_None);
			return Py_None;
		}
	return NULL;
}

PyObject *CORBA__TypeCode(PyObject *self, PyObject *args)
{
	return (PyObject *)CORBA_TypeCode_PyObject__new(args);
}

PyObject *CORBA__fixed(PyObject *self, PyObject *args)
{
	return (PyObject *)CORBA_fixed_PyObject__new(args);
}

PyObject *CORBA__Any(PyObject *self, PyObject *args)
{
	return (PyObject *)CORBA_any_PyObject__new(args);
}

void init_consts()
{
	PyDict_SetItemString(ModuleDict, "TRUE", PyInt_FromLong(1));
	PyDict_SetItemString(ModuleDict, "FALSE", PyInt_FromLong(0));
	PyDict_SetItemString(ModuleDict, "ORB_ID", 
	                     PyString_FromString("orbit-local-orb"));
}


PyMethodDef CORBA_methods[] = {
	{ "ORB_init", CORBA__ORB_init, 1 },
	{ "Object_cast", CORBA__Object_cast, 1 },
	{ "load_idl", CORBA__load_idl, 1 },
	{ "TypeCode", CORBA__TypeCode, 1 },
	{ "Any", CORBA__Any, 1 },
	{ "fixed", CORBA__fixed, 1 },
	{ NULL, NULL }
};

void initCORBA()
{
	PyObject *m, *c;
	CORBA_PyObject_Type.ob_type = &PyType_Type;
	CORBA_ORB_PyObject_Type.ob_type = &PyType_Type;

	m = Py_InitModule("CORBA", CORBA_methods);
	ModuleDict = PyModule_GetDict(m);

	// FIXME: I would really like to get the scope that imports the
	// CORBA module.  But how?
	main_module = PyImport_ImportModule("__main__");
	MainDict = PyModule_GetDict(main_module);

	object_glue = g_hash_table_new(g_str_hash, g_str_equal);
	exceptions = g_hash_table_new(g_str_hash, g_str_equal);
	init_typecodes();
	init_exceptions();
	init_server();
	init_consts();

	// Make the POA frame class
	c = PyClass_New(NULL, PyDict_New(), PyString_FromString("POA"));
	poa_instances = c;
	PyDict_SetItemString(MainDict, "POA", c);

	// The Portable Server stuff
	POAManager_PyObject_Type.ob_type = &PyType_Type;
   POA_PyObject_Type.ob_type = &PyType_Type;
   m = Py_InitModule("PortableServer", PortableServer_methods);
	PyObject_SetAttrString(main_module, "PortableServer", m);
}
