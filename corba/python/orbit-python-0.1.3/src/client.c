#include <orb/interface_repository.h>
#include <Python.h>
#include "CORBAmodule.h"

typedef struct {
	CORBA_unsigned_long len;
	char opname[1];
} OpNameData;

GPtrArray *marshal_call(CORBA_Object obj, GIOPConnection *con,
                        GIOP_unsigned_long req_id,
                        CORBA_OperationDescription *opd, PyObject *args)
{
	OpNameData *ond;
	static struct iovec ovec;
	GIOPSendBuffer *send_buffer = NULL;
	GPtrArray *return_types = g_ptr_array_new();
	CORBA_unsigned_long i = 0, cur = 0;

	ond = (OpNameData *)g_malloc(sizeof(OpNameData) + strlen(opd->name));
	ond->len = strlen(opd->name) + 1;
	strcpy(ond->opname, opd->name);
	ovec.iov_base = (char *)ond;
	ovec.iov_len = sizeof(CORBA_unsigned_long) + ond->len;

	send_buffer = giop_send_request_buffer_use(con, NULL, req_id,
	                   CORBA_TRUE, &obj->active_profile->object_key_vec,
	                   &ovec, &ORBit_default_principal_iovec);

	if (!send_buffer) {
		PyErr_SetString(ErrorObject, "IDL:omg.org/CORBA/COMM_FAILURE:1.0");
		goto exception;
	}

	// Now we marshal and stuff the return types into an array for
	// demarshalling.
	if (opd->result->kind != CORBA_tk_void) 
		g_ptr_array_add(return_types, opd->result);

	for (i = 0, cur = 0; i < opd->parameters._length; i++) {
		PyObject *a;
		CORBA_TypeCode tc = opd->parameters._buffer[i].type;
		switch (opd->parameters._buffer[i].mode) {

			case CORBA_PARAM_IN:
				a = PyTuple_GetItem(args, cur);
				if (!marshal_arg(a, send_buffer, tc)) 
					goto exception;
				cur++;
				break;

			case CORBA_PARAM_INOUT:
				a = PyTuple_GetItem(args, cur);
				if (!marshal_arg(a, send_buffer, tc)) 
					goto exception;

			case CORBA_PARAM_OUT:
				g_ptr_array_add(return_types, tc);
				cur++;
				break;
		}
	}
	// still need to handle attribute stuff
	
	giop_send_buffer_write(send_buffer);

exception:
	giop_send_buffer_unuse(send_buffer);
	g_free(ond);

	return return_types;
}
	
GIOPConnection *demarshal_call(CORBA_Object obj, GIOPConnection *con,
                          GIOP_unsigned_long req_id,
                          CORBA_OperationDescription *opd,
                          PyObject *args,
                          GPtrArray *return_types, PyObject **tuple)
{
	GIOPRecvBuffer *buf;
	CORBA_unsigned_long i;

	buf = giop_recv_reply_buffer_use_2(con, req_id, TRUE);
	if (!buf) {
		PyErr_SetString(ErrorObject, "IDL:omg.org/CORBA/COMM_FAILURE:1.0");
		goto exception;
	}

	// TODO: figure out WTF this does.
	if (buf->message.u.reply.reply_status == GIOP_LOCATION_FORWARD) {
		if (obj->forward_locations != NULL)
			ORBit_delete_profiles(obj->forward_locations);

		obj->forward_locations = ORBit_demarshal_IOR(buf);
		con = ORBit_object_get_forwarded_connection(obj);
		giop_recv_buffer_unuse(buf);
		return con;
	}
	else if (buf->message.u.reply.reply_status != GIOP_NO_EXCEPTION) {
		demarshal_exception(buf, NULL, (CORBA_exception_type)
		                    buf->message.u.reply.reply_status, opd);
		goto exception;
	}
	// We are ready to demarshal now
	*tuple = PyTuple_New(return_types->len);
	for (i = 0; i < return_types->len; i++) {
		CORBA_TypeCode tc = (CORBA_TypeCode)return_types->pdata[i];
		PyObject *o = demarshal_arg(buf, tc);
		if (!o) 
			goto exception;
		PyTuple_SetItem(*tuple, i, o);
	}
	
exception:
//	if (PyErr_Occurred())
//		PyErr_Print();
	g_ptr_array_free(return_types, TRUE);
	giop_recv_buffer_unuse(buf);
	return NULL;	
}
		

PyObject *_stub_func(CORBA_PyObject *self, PyObject *args,
                     CORBA_OperationDescription *opd)
{
	GIOPConnection *con;
	GIOP_unsigned_long req_id;
	GPtrArray *r = NULL;
	PyObject *tuple = NULL;
	// Figure out how many arguments we need (total args - # OUT args)
	int need_args = opd->parameters._length, i;
	for (i = 0; i < opd->parameters._length; i++)
		if (opd->parameters._buffer[i].mode == CORBA_PARAM_OUT)
			need_args--;
		
	if (PyTuple_Size(args) != need_args) {
		PyErr_Format(PyExc_TypeError, 
		   "not enough arguments; expected %d, got %d", 
		    need_args, PyTuple_Size(args));
		goto bail;
	}

	con = ORBit_object_get_connection(self->obj);

retry_request:
	req_id = GPOINTER_TO_UINT(alloca(0));
	r = marshal_call(self->obj, con, req_id, opd, args);

	// Demarshal the results if there are any
	if (opd->mode == CORBA_OP_ONEWAY && r->len)
		g_warning("ONEWAY operation has output parameters!");
	else if (opd->mode != CORBA_OP_ONEWAY && !PyErr_Occurred()) {
		GIOPConnection *newcon;
		newcon  = demarshal_call(self->obj, con, req_id, opd, args, r, &tuple);
		if (newcon) {
			con = newcon;
			goto retry_request;
		}
	}
		                      
bail:
	if (PyErr_Occurred())
		return NULL;

	if (!tuple) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (PyTuple_Size(tuple) == 1) {
		// don't return a tuple for just one return value
		PyObject *o = PyTuple_GetItem(tuple, 0);
		Py_INCREF(o);
		// XXX: Calling Py_XDECREF on this tuple causes instability.  But why?!
		// I think I fixed this, but I'll leave this comment here just in case.
		Py_XDECREF(tuple);
		return o;
	}
	return tuple;
}

PyObject *stub_func(CORBA_PyObject *self, PyObject *args)
{
   CORBA_PyObject_Glue *glue;
	int index = -1;
	PyObject *r;
	CORBA_OperationDescription *opd;
	// pop an item from the last_method stack
	char *last_method = (char *)self->last_method->data;
	self->last_method = g_slist_remove_link(self->last_method, 
	                                        self->last_method);
   glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue,self->repo_id);
	opd = find_operation(glue, last_method);
	g_free(last_method);
	if (!opd) {
		PyErr_Format(ErrorObject, 
		                "Interface %s not registered.", self->repo_id);
		return NULL;
	}
	r = _stub_func(self, args, opd);
//	Py_INCREF(Py_None); return Py_None;
	return r;

}

PyObject *get_attribute(CORBA_PyObject *self, CORBA_AttributeDescription *ad)
{
	// construct a dummy op for fetching this attribute
	// FIXME: no reason why we couldn't make this persistent

	CORBA_OperationDescription *opd = g_new0(CORBA_OperationDescription, 1);
	PyObject *args, *result;
	opd->name = g_strconcat("_get_", ad->name, NULL);
	opd->result = ad->type;
	opd->parameters._length = 0;

	// make a dummy tuple of size 0
	args = PyTuple_New(0);
	result = _stub_func(self, args, opd);
	g_free(opd);
	Py_XDECREF(args);
	if (!result) {
		g_warning("Apparently get attribute failed");
	}
	return result;
}

int set_attribute(CORBA_PyObject *self, CORBA_AttributeDescription *ad,
                  PyObject *value)
{
	CORBA_OperationDescription *opd;
	PyObject *args, *result;

	// Make sure we aren't trying to set a readonly attribute
	// FIXME: what's the proper exception to throw?
	if (ad->mode == CORBA_ATTR_READONLY) {
		raise_system_exception("IDL:omg.org/CORBA/NO_PERMISSION:1.0", 0,
		                       CORBA_COMPLETED_NO, "attribute %s readonly",
		                       ad->name);
		return -1;
	}

	// construct a dummy op for setting this attribute
	// FIXME: no reason why we couldn't make this persistent

	opd = g_new0(CORBA_OperationDescription, 1);
	opd->name = g_strconcat("_set_", ad->name, NULL);
	opd->result = dupe_tc(TC_void);
	opd->parameters._length = 1;

	// set up the parameter
	opd->parameters._buffer = 
	   CORBA_sequence_CORBA_ParameterDescription_allocbuf(1);
	opd->parameters._buffer[0].name = ad->name;
	opd->parameters._buffer[0].type = ad->type;
	opd->parameters._buffer[0].mode = CORBA_PARAM_IN;
	opd->parameters._buffer[0].type_def = CORBA_OBJECT_NIL;

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, value);
	Py_XINCREF(value);
	result = _stub_func(self, args, opd);
	// FIXME: how do I free opd->parameters._buffer?  I am leaking memory here.
	//	ORBit_free(opd->parameters._buffer, CORBA_FALSE);
	g_free(opd);
	Py_XDECREF(args);
	if (!result)
		return -1;
	Py_XDECREF(result);
	PyErr_Clear();
	return 0;
}


