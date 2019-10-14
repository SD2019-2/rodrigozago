#include "CORBAmodule.h"

/***************************************************************************
	Wraps POAManager
	Methods handled:
		activate
*/


PyTypeObject POAManager_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"POAManager",         /*tp_name*/
	sizeof(POAManager_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)POAManager_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)POAManager_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)POAManager_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	0,       /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

#define POAManager_PyObject_Check(v) ((v)->ob_type == &POAManager_PyObject_Type)

POAManager_PyObject *POAManager_Object_to_PyObject(PortableServer_POAManager o)
{
	POAManager_PyObject *self;
	self = PyObject_NEW(POAManager_PyObject, &POAManager_PyObject_Type);
	if (!self)
		return NULL;
	CORBA_exception_init(&self->ev);
	self->obj = o;
	return self;
}

void POAManager_PyObject__dealloc(POAManager_PyObject *self)
{
	PyMem_DEL(self);
}

PyObject *POAManager_PyObject__activate(POAManager_PyObject *self, 
	                                     PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	PortableServer_POAManager_activate(self->obj, &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

PyMethodDef POAManager_PyObject_methods[] = {
	{ "activate", (PyCFunction)POAManager_PyObject__activate, 1 },
	{ NULL, NULL }
};

PyObject *POAManager_PyObject__getattr(POAManager_PyObject *self, char *name)
{
	return Py_FindMethod(POAManager_PyObject_methods, (PyObject *)self, name);
}

int POAManager_PyObject__setattr(POAManager_PyObject *self, char *name,
	                              PyObject *v)
{
	return -1;
}

/***************************************************************************
	Wraps POA
	Methods handled:
*/

PyTypeObject POA_PyObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,       /*ob_size*/
	"PortableServer.POA",         /*tp_name*/
	sizeof(POA_PyObject),   /*tp_basicsize*/
	0,       /*tp_itemsize*/
	/* methods */
	(destructor)POA_PyObject__dealloc, /*tp_dealloc*/
	0,       /*tp_print*/
	(getattrfunc)POA_PyObject__getattr, /*tp_getattr*/
	(setattrfunc)POA_PyObject__setattr, /*tp_setattr*/
	0,       /*tp_compare*/
	0,       /*tp_repr*/
	0,       /*tp_as_number*/
	0,       /*tp_as_sequence*/
	0,       /*tp_as_mapping*/
	0,       /*tp_hash*/
};

#define POA_PyObject_Check(v) ((v)->ob_type == &POA_PyObject_Type)

POA_PyObject *POA_Object_to_PyObject(PortableServer_POA object)
{
	POA_PyObject *self;
	self = PyObject_NEW(POA_PyObject, &POA_PyObject_Type);
	if (!self)
		return NULL;
	CORBA_exception_init(&self->ev);
	self->obj = object;
	return self;
}

void POA_PyObject__dealloc(POA_PyObject *self)
{
	PyMem_DEL(self);
}

PyObject *POA_PyObject__activate_object(POA_PyObject *self, PyObject *args)
{
	PortableServer_ObjectId *id;
	Servant_PyObject *servant;
	if (!PyArg_ParseTuple(args, "O", &servant))
		return NULL;
	// FIXME: validate that this object is a servant
	id = PortableServer_POA_activate_object(self->obj, servant->servant,
	                                        &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;

	servant->activated = CORBA_TRUE;
	servant->poa = self;
	servant->oid = id;
	PyObject_SetAttrString(servant->servant->impl, "_servant", 
	                       (PyObject *)servant);

	return Py_BuildValue("s#", id->_buffer, id->_length);
}

PyObject *POA_PyObject__activate_object_with_id(POA_PyObject *self,
	                                             PyObject *args)
{
	Servant_PyObject *servant;
	PortableServer_ObjectId *id;
	id = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();
	if (!PyArg_ParseTuple(args, "s#O", &id->_buffer, &id->_length, &servant))
		return NULL;
	PortableServer_POA_activate_object_with_id(self->obj, id, servant->servant,
	                                           &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;

	servant->activated = CORBA_TRUE;
	servant->poa = self;
	servant->oid = id;
	Py_INCREF(Py_None);
	return Py_None;
}


PyObject *POA_PyObject__deactivate_object(POA_PyObject *self, PyObject *args)
{
	PortableServer_ObjectId *id;
	Servant_PyObject *servant;
	if (!PyArg_ParseTuple(args, "O", &servant))
		return NULL;

	// FIXME: validate that this object is a servant
	if (servant->activated) {
		PortableServer_POA_deactivate_object(self->obj, servant->oid, &self->ev);
		if (!check_corba_ex(&self->ev))
			return NULL;
		servant->activated = CORBA_FALSE;
		Py_DECREF(servant->servant->impl);
		CORBA_free(servant->oid);
	}
	else {
		g_warning("deactivate_object called on inactive object");
	}
	Py_INCREF(Py_None);
	return Py_None;
}

// ORBit hasn't implemented this yet.
PyObject *POA_PyObject__create_reference(POA_PyObject *self, PyObject *args)
{
	PortableServer_ObjectId *id;
	char *repo_id;
	CORBA_Object new_obj;

	if (!PyArg_ParseTuple(args, "s", &repo_id))
		return NULL;
	new_obj = PortableServer_POA_create_reference(self->obj,
	                                        repo_id, &self->ev);
	return (PyObject *)CORBA_Object_to_PyObject(new_obj);
}

PyObject *POA_PyObject__servant_to_reference(POA_PyObject *self, PyObject *args)
{
	CORBA_Object ref = CORBA_OBJECT_NIL;
	PyObject *tmp, *ret;
	Servant_PyObject *servant;

	if (!PyArg_ParseTuple(args, "O", &servant))
		return NULL;

	// XXX: raise an exception instead
	if (!servant->activated) {
		g_warning("Servant must be activated first!");
		Py_INCREF(Py_None);
		return Py_None;
	}
		
	ref = PortableServer_POA_servant_to_reference(self->obj, servant->servant,
	                                              &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;
	if (ref == CORBA_OBJECT_NIL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return (PyObject *)CORBA_Object_to_PyObject(ref);
}

PyObject *POA_PyObject__reference_to_servant(POA_PyObject *self, PyObject *args)
{
	CORBA_Object obj;
	PyObject *tmp, *ret;
	CORBA_PyObject *ref;

	if (!PyArg_ParseTuple(args, "O", &ref))
		return NULL;
	obj = (CORBA_Object)PortableServer_POA_reference_to_servant(self->obj, 
			              ref->obj, &self->ev);
	if (!check_corba_ex(&self->ev))
		return NULL;

	if (ref == CORBA_OBJECT_NIL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	Py_INCREF(Py_None);
	return Py_None;
}
PyMethodDef POA_PyObject_methods[] = {
	{ "activate_object", (PyCFunction)POA_PyObject__activate_object, 1 },
	{ "activate_object_with_id", 
	           (PyCFunction)POA_PyObject__activate_object_with_id, 1 },
	{ "deactivate_object", (PyCFunction)POA_PyObject__deactivate_object, 1 },
	{ "create_reference", (PyCFunction)POA_PyObject__create_reference, 1 },
	{ "servant_to_reference", 
	           (PyCFunction)POA_PyObject__servant_to_reference, 1 },
	{ "reference_to_servant", 
	           (PyCFunction)POA_PyObject__reference_to_servant, 1 },
	{ NULL, NULL }
};

PyObject *POA_PyObject__getattr(POA_PyObject *self, char *name)
{
	if (!strcmp(name, "the_POAManager")) {
		PortableServer_POAManager m;
		m = PortableServer_POA__get_the_POAManager(self->obj, &self->ev);
		if (!check_corba_ex(&self->ev))
			return NULL;
		if (m == CORBA_OBJECT_NIL) {
			Py_INCREF(Py_None);
			return Py_None;
		}
		return (PyObject *)POAManager_Object_to_PyObject(m);
	}
	return Py_FindMethod(POA_PyObject_methods, (PyObject *)self, name);
}

int POA_PyObject__setattr(POA_PyObject *self, char *name, PyObject *v)
{
	return -1;
}

PyObject *RootPOA(PyObject *self, PyObject *arg)
{
	CORBA_ORB_PyObject *orb;
	PortableServer_POA poa;
	if (PyArg_ParseTuple(arg, "O", &orb)) {
		poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb->obj,
	                "RootPOA", &orb->ev);
		if (!check_corba_ex(&orb->ev))
			return NULL;
		if (poa != CORBA_OBJECT_NIL)
			return (PyObject *)POA_Object_to_PyObject(poa);
	}
	return NULL;
}

	
PyMethodDef PortableServer_methods[] = {
	{ "RootPOA", RootPOA, 1 },
	{ NULL, NULL }
};
