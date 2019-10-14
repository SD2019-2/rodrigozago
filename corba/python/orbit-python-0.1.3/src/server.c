#include "CORBAmodule.h"

void init_server()
{
}

void Servant_PyObject__dealloc(Servant_PyObject *self)
{
	Py_DECREF(self->servant->impl);
	g_free(self->servant);
	g_free(self->class_info);
	PyMem_DEL(self);
}

PyObject *Servant_PyObject__getattr(Servant_PyObject *self, char *name)
{
	PyObject *impl = self->servant->impl;
	return PyObject_GetAttrString(impl, name);
}

int Servant_PyObject__setattr(Servant_PyObject *self, char *name, PyObject *v)
{
	PyObject *impl = self->servant->impl;
	return PyObject_SetAttrString(impl, name, v);
}

CORBA_OperationDescription *find_operation(CORBA_PyObject_Glue *glue, char *n)
{
	CORBA_unsigned_long i;
	CORBA_InterfaceDef_FullInterfaceDescription *d;

	for (i = 0; i < glue->desc->operations._length; i++) {
		if (!strcmp(n, glue->desc->operations._buffer[i].name))
			return &glue->desc->operations._buffer[i];
	}
	// Look through base interfaces for this operation
	d = glue->desc;
	for (i = 0; i < d->base_interfaces._length; i++) {
		char *repo_id = d->base_interfaces._buffer[i];
		glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue, repo_id);
		if (glue) {
			CORBA_OperationDescription *res = find_operation(glue, n);
			if (res)
				return res;
		}
	}
	return NULL;
}

CORBA_AttributeDescription *find_attribute(CORBA_PyObject_Glue *glue, char *n)
{
	CORBA_unsigned_long i;
	CORBA_InterfaceDef_FullInterfaceDescription *d;
	for (i = 0; i < glue->desc->attributes._length; i++) {
		if (!strcmp(n, glue->desc->attributes._buffer[i].name))
			return &glue->desc->attributes._buffer[i];
	}
	// Look through base interfaces for this attribute
	d = glue->desc;
	for (i = 0; i < d->base_interfaces._length; i++) {
		char *repo_id = d->base_interfaces._buffer[i];
		glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue, repo_id);
		if (glue) {
			CORBA_AttributeDescription *res = find_attribute(glue, n);
			if (res)
				return res;
		}
	}
	return NULL;
}
void operation_skel(PyOrbit_Servant *servant, GIOPRecvBuffer *buf,
                    CORBA_Environment *ev, gpointer impl)
{
	GIOPSendBuffer *sb = NULL;
	CORBA_OperationDescription *opd = (CORBA_OperationDescription *)impl;
	CORBA_unsigned_long i, cur = 0;
	PyObject *result = NULL, *tuple, *dict;
	CORBA_boolean is_attr_op = CORBA_FALSE;

	// Figure out how many paramters we're passing to the python function
	for (i = 0, cur = 0; i < opd->parameters._length; i++)
		if (opd->parameters._buffer[i].mode != CORBA_PARAM_OUT)
			cur++;

	// Construct the parameter list to pass to the python method
	tuple = PyTuple_New(cur);
	for (i = 0, cur = 0; i < opd->parameters._length; i++)
		if (opd->parameters._buffer[i].mode != CORBA_PARAM_OUT) {
			PyObject *arg = demarshal_arg(buf, opd->parameters._buffer[i].type);
			PyTuple_SetItem(tuple, cur++, arg);
		}
	// Fetch an attribute
	dict = ((PyInstanceObject *)servant->impl)->in_dict;
	if (!strncmp(opd->name, "_get_", 5)) {
		is_attr_op = CORBA_TRUE;
		result = PyDict_GetItemString(dict, &opd->name[5]);
		// FIXME: To prevent the client from hanging, we should probably
		// return some default value here.
		if (!result)
			PyErr_SetString(ErrorObject, "Attribute not yet defined");
	}
	// set an attribute
	else if (!strncmp(opd->name, "_set_", 5)) {
		is_attr_op = CORBA_TRUE;
		result = PyTuple_GetItem(tuple, 0);
		Py_INCREF(result);
		PyDict_SetItemString(dict, &opd->name[5], result);
		Py_INCREF(Py_None);
		result = Py_None;
	}
	else 
		result = PyObject_CallMethod(servant->impl, opd->name, "O", tuple);
	Py_DECREF(tuple);
	if (opd->mode == CORBA_OP_ONEWAY)
		goto bail;

	// Make the return value a tuple if it isn't
	if (is_attr_op || result && !PyTuple_Check(result)) {
		int size = result == Py_None ? 0 : 1;
		tuple = PyTuple_New(size);
		if (size)
			PyTuple_SetItem(tuple, 0, result);
		result = tuple;
	}
	else if (result && PyTuple_Check(result))
		tuple = result;

	sb = giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(buf)->connection, NULL,
	                                buf->message.u.request.request_id,
	                                (GIOPReplyStatusType)CORBA_NO_EXCEPTION);
	// Check to see if the method raised an exception
	if (PyErr_Occurred()) {
		PyObject *type, *val, *tb;
		CORBA_exception_type ex_type;

		PyErr_Fetch(&type, &val, &tb);
		// TODO: pass on unknown exceptions
		ex_type = marshal_exception(type, val, sb, NULL, opd);
		if (ex_type == CORBA_NO_EXCEPTION) {
			// repost the exception
			g_warning("marshal_exception failed");
			PyErr_Restore(type, val, tb);
		}
		sb->message.u.reply.reply_status = (GIOPReplyStatusType)ex_type;
		goto send;
	}

	// Marshal the return type
	cur = 0;
	if (opd->result->kind != CORBA_tk_void) {
		PyObject *arg = PyTuple_GetItem(tuple, cur++);
		if (!marshal_arg(arg, sb, opd->result))
			goto bail;
	}

	// Marshal the out/inout parameters
	for (i = 0; i < opd->parameters._length; i++) {
		CORBA_ParameterDescription *pd = &opd->parameters._buffer[i];
		if (pd->mode == CORBA_PARAM_OUT || pd->mode == CORBA_PARAM_INOUT) {
			PyObject *arg;
			if (cur == PyTuple_Size(result)) {
				// XXX: more useful error message?
				PyErr_SetString(ErrorObject, "not enough values returned");
				goto bail;
			}
			arg = PyTuple_GetItem(tuple, cur++);
			if (!marshal_arg(arg, sb, pd->type))
				goto bail;
		}
	}
send:
	giop_send_buffer_write(sb);

bail:
	if (PyErr_Occurred())
		PyErr_Print();
//	if (result)
//		Py_DECREF(result);
	if (sb) {
		giop_send_buffer_unuse(sb);
		sb = NULL;
	}
	// free the dummy operation descriptions
	if (!strncmp(opd->name, "_get_", 5))
		g_free(opd);
	else if (!strncmp(opd->name, "_set_", 5)) {
		// FIXME: leak: how do I free opd->parameters._buffer?
		g_free(opd);
	}
}
      

static ORBitSkeleton get_skel(PyOrbit_Servant *self, GIOPRecvBuffer *buf,
                                   gpointer *impl)
{
   gchar *opname = buf->message.u.request.operation;

	CORBA_OperationDescription *opd = find_operation(self->glue, opname);
	if (!opd) {
		// Fetch the description for this attribute
		char *attr = &opname[5];
		CORBA_AttributeDescription *ad = find_attribute(self->glue, attr);
		// We abort if this attribute doesn't exist.		
		if (!ad)
			return (ORBitSkeleton)NULL;

		// We're good to go; create a dummy description for the operation
		opd = g_new0(CORBA_OperationDescription, 1);
		opd->name = opname;
		if (!strncmp(opname, "_get_", 5)) {
			opd->result = ad->type;
			opd->parameters._length = 0;
		}
		else if (!strncmp(opname, "_set_", 5)) {
			opd->result = dupe_tc(TC_void);
			opd->name = opname;
			opd->parameters._length = 1;
			opd->parameters._buffer = 
			   CORBA_sequence_CORBA_ParameterDescription_allocbuf(1);
			opd->parameters._buffer[0].name = ad->name;
			opd->parameters._buffer[0].type = ad->type;
			opd->parameters._buffer[0].mode = CORBA_PARAM_IN;
			opd->parameters._buffer[0].type_def = CORBA_OBJECT_NIL;
		} else {
			g_warning("unhandled event");
			return 0;
		}
		
	}
	*impl = opd;
	return (ORBitSkeleton)operation_skel;
}


PyObject *new_poa_servant(PyObject *repo_object, PyObject *args)
{
	PyObject *impl;
	char *s;
	CORBA_PyObject_Glue *glue;
	Servant_PyObject *o;
	CORBA_Environment ev;
	PyOrbit_Servant *servant;

	if (!PyArg_ParseTuple(args, "O!", &PyInstance_Type, &impl)) 
		return NULL;
	s = PyString_AsString(repo_object);

	glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue, s);

	o = PyObject_NEW(Servant_PyObject, glue->servant_type);
	o->activated = CORBA_FALSE;
	o->class_info = g_new(PortableServer_ClassInfo, 1);
	servant = g_new0(PyOrbit_Servant, 1);

	CORBA_exception_init(&ev);

	PortableServer_ServantBase__init((PortableServer_ServantBase *)servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		g_error("Exception while initializing servant");
	o->class_info->relay_call = (ORBit_impl_finder)get_skel;
	o->class_info->class_name = g_strdup(glue->desc->id);
	o->class_info->init_local_objref = NULL;
	o->class_id = ORBit_register_class(o->class_info);

	ORBIT_OBJECT_KEY(servant->_private)->class_info = o->class_info;

	o->servant = servant;
	Py_INCREF(impl);
	servant->impl = impl;
	servant->glue = glue;

	return (PyObject *)o;
}
