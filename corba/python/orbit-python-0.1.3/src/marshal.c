#include <Python.h>
#include <orb/interface_repository.h>
#include "CORBAmodule.h"
#include "types.h"

#define buf_putn giop_send_buffer_append_mem_indirect_a

CORBA_boolean marshal_short(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_short v;
	if (!PyInt_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected integer, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "h", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}

CORBA_boolean marshal_boolean(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_short v;
	if (!PyInt_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected integer, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "h", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}
	
// FIXME: check typecode for length
CORBA_boolean marshal_string(PyObject *arg, GIOPSendBuffer *buf)
{
	char *s;
	CORBA_unsigned_long len;

	if (!PyString_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected string, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "s", &s))
		return CORBA_FALSE;

	len = strlen(s) + 1;
	buf_putn(buf, &len, sizeof(len));
	giop_send_buffer_append_mem_indirect(buf, s, len);
	return CORBA_TRUE;
}

// Make a TypeCode for system exceptions, borrowed from CORBA::ORBit
static const char *Status_SubNames[] = {
	"COMPLETED_YES", "COMPLETED_NO", "COMPLETED_MAYBE"
};
static struct CORBA_TypeCode_struct Status_TypeCode = {
	{}, CORBA_tk_enum, NULL, NULL, 0, 3, Status_SubNames
};
static const char *SystemExcept_SubNames[] = { "-minor", "-status" };
static CORBA_TypeCode SystemExcept_SubTypes[] = {
	(CORBA_TypeCode)TC_CORBA_ulong, &Status_TypeCode
};
static struct CORBA_TypeCode_struct SystemExcept_TypeCode = {
	{}, CORBA_tk_except, NULL, NULL, 0, 2, SystemExcept_SubNames,
	SystemExcept_SubTypes
};

CORBA_exception_type marshal_exception(PyObject *type, PyObject *data,
                                       GIOPSendBuffer *buf, CORBA_TypeCode tc,
                                       CORBA_OperationDescription *opd)
{
	PyObject *repo_object, *base;
	char *repo_id;
	CORBA_unsigned_long i, len;

	g_return_val_if_fail (data != 0 && type != 0 && buf != 0,
	                      CORBA_NO_EXCEPTION);

	repo_object = PyObject_GetAttrString(type, "__repo_id");
	if (!repo_object) {
		g_warning("Can't fetch repository id for this exception");
		raise_system_exception("IDL:omg.org/CORBA/INTERNAL:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, NULL);
		return CORBA_NO_EXCEPTION;
	}
	PyArg_Parse(repo_object, "s", &repo_id);
	base = PyTuple_GetItem(((PyClassObject *)type)->cl_bases, 0);

	if (base != UserExcept && base != SystemExcept) {
		g_warning("Exception must be derived from CORBA.UserException");
		return CORBA_NO_EXCEPTION;
	}
	if (base == SystemExcept) {
		g_warning("Handle SystemException here");
		tc = &SystemExcept_TypeCode;
	}

	if (!tc && opd)
		for (i = 0; i < opd->exceptions._length; i++) {
			if (!strcmp(opd->exceptions._buffer[i].id, repo_id)) {
				tc = opd->exceptions._buffer[i].type;
				break;
			}
		}
	if (!tc) {
		g_warning("Unknown exception received");
		raise_system_exception("IDL:omg.org/CORBA/UNKNOWN:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, NULL);
		return CORBA_NO_EXCEPTION;
	}
	len = strlen(repo_id) + 1;
	buf_putn(buf, &len, sizeof(len));
	giop_send_buffer_append_mem_indirect(buf, repo_id, len);

	if (tc->sub_parts) {
		if (!PyInstance_Check(data)) {
			g_warning("Exception data must be an instance");
			return CORBA_NO_EXCEPTION;
		}
		for (i = 0; i < tc->sub_parts; i++) {
			PyObject *val = PyObject_GetAttrString(data, (char *)tc->subnames[i]);
			if (!val) { // FIXME: throw an exception
				g_warning("Missing exception member %s", tc->subnames[i]);
				return CORBA_NO_EXCEPTION;
			}

			if (!marshal_arg(val, buf, tc->subtypes[i]))
				return CORBA_NO_EXCEPTION;
		}
	}
	return base == UserExcept ? CORBA_USER_EXCEPTION : CORBA_SYSTEM_EXCEPTION;
}

CORBA_boolean marshal_object(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_Object obj = CORBA_OBJECT_NIL;
	if (arg) {
		obj = ((CORBA_PyObject *)arg)->obj;
	}
	// XXX: Check type of obj?
	ORBit_marshal_object(buf, obj);
	return CORBA_TRUE;
}

CORBA_boolean marshal_struct(PyObject *arg, GIOPSendBuffer *buf,
                             CORBA_TypeCode tc)
{
	CORBA_unsigned_long i;
	for (i = 0; i < tc->sub_parts; i++) {
		PyObject *val = PyObject_GetAttrString(arg, (char *)tc->subnames[i]);
		// FIXME: throw an exception here
		if (!val) {
			g_warning("Missing struct member %s", tc->subnames[i]);
			return CORBA_FALSE;
		}
		if (!marshal_arg(val, buf, tc->subtypes[i]))
			return CORBA_FALSE;
	}
	return CORBA_TRUE;
}

CORBA_boolean marshal_array(PyObject *arg, GIOPSendBuffer *buf,
                            CORBA_TypeCode tc)
{
	PyObject *tuple;
	CORBA_unsigned_long i;
	if (PyList_Check(arg))
		tuple = PyList_AsTuple(arg);
	else if (PyTuple_Check(arg))
		tuple = arg;
	else { // FIXME: raise an exception here
		g_warning("Array type must be either list or tuple");
		return CORBA_FALSE;
	}
	if (PyTuple_Size(tuple) != tc->length) { // FIXME: raise exception
		g_warning("Sequence length must be length %d", tc->length);
		return CORBA_FALSE;
	}
	for (i = 0; i < tc->length; i++)
		if (!marshal_arg(PyTuple_GetItem(tuple, i), buf, tc->subtypes[0]))
			return CORBA_FALSE; 
	return CORBA_TRUE;
}

CORBA_boolean marshal_sequence(PyObject *arg, GIOPSendBuffer *buf,
                               CORBA_TypeCode tc)
{
	PyObject *tuple;
	CORBA_unsigned_long i, len;

	// TODO: handle character sequences as strings
	if (PyList_Check(arg))
		tuple = PyList_AsTuple(arg);
	else if (PyTuple_Check(arg))
		tuple = arg;
	else { // FIXME: raise an exception here
		g_warning("Sequence type must be either list or tuple");
		return CORBA_FALSE;
	}
	// FIXME: raise an exception
	if (tc->length && PyTuple_Size(tuple) > tc->length) {
		g_warning("Sequence length exceeds bounds");
		return CORBA_FALSE;
	}
	len = PyTuple_Size(tuple);
	buf_putn(buf, &len, sizeof(len));
	for (i = 0; i < len; i++) {
		if (!marshal_arg(PyTuple_GetItem(tuple, i), buf, tc->subtypes[0]))
			return CORBA_FALSE; 
	}

	return CORBA_TRUE;
}

CORBA_boolean marshal_enum(PyObject *arg, GIOPSendBuffer *buf,
                           CORBA_TypeCode tc)
{
	CORBA_unsigned_long v;
	// FIXME: raise exception
	if (!PyInt_Check(arg)) {
		g_message("Enum element must be an integer value");
		return CORBA_FALSE;
	}
	v = PyInt_AsLong(arg);
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}

CORBA_boolean marshal_union(PyObject *arg, GIOPSendBuffer *buf,
                           CORBA_TypeCode tc)
{
	PyObject *v = PyObject_GetAttrString(arg, "v"),
	         *d = PyObject_GetAttrString(arg, "d");
	CORBA_long arm;

	if (!v || !d || v == Py_None || d == Py_None) {
		// FIXME: raise exception
		g_warning("Both value and discriminator must be set!");
		return CORBA_FALSE;
	}
	arm = find_union_arm(tc, d);
	if (arm < 0) {
		// FIXME: raise exception
		g_warning("Unknown discriminator in union");
		return CORBA_FALSE;
	}
	if (!marshal_arg(d, buf, tc->discriminator))
		return CORBA_FALSE;
	return marshal_arg(v, buf, tc->subtypes[arm]);
}

	
CORBA_boolean marshal_long(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_long v;
	if (!PyInt_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected integer, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "l", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}
	
CORBA_boolean marshal_char(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_char v;
	if (!PyString_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected string, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "c", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, 1);
	return CORBA_TRUE;
}

CORBA_boolean marshal_octet(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_octet v;
	if (!PyInt_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected integer, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "b", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}


CORBA_boolean marshal_float(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_float v;
	if (!PyFloat_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected float, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "f", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}

CORBA_boolean marshal_double(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_double v;
	if (!PyFloat_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected float, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "d", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}

CORBA_boolean marshal_longlong(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_long_long v;
	if (!PyInt_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected integer, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	if (!PyArg_Parse(arg, "l", &v))
		return CORBA_FALSE;
	buf_putn(buf, &v, sizeof(v));
	return CORBA_TRUE;
}

CORBA_boolean marshal_any(PyObject *arg, GIOPSendBuffer *buf)
{
	CORBA_TypeCode_PyObject *tco;
	CORBA_TypeCode tc;

	if (!CORBA_any_PyObject_Check(arg)) {
		raise_system_exception("IDL:omg.org/CORBA/BAD_PARAM:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, "Expected any, got %s", 
		                       arg->ob_type->tp_name);
		return CORBA_FALSE;
	}
	tco = (CORBA_TypeCode_PyObject *)((CORBA_any_PyObject *)arg)->tc_object;
	tc = tco->tc;
	ORBit_encode_CORBA_TypeCode(tc, buf);
	return marshal_arg(((CORBA_any_PyObject *)arg)->value, buf, tc);
}

CORBA_boolean marshal_arg(PyObject *arg, GIOPSendBuffer *buf,
                          CORBA_TypeCode tc)
{
	CORBA_PyObject_Glue *glue;
	d_message(2, "marshal_arg: tc->kind = %d, tc->name = %s",
	          tc->kind, tc->name);
	if (!arg) {
		g_warning("marshal_arg: (arg == NULL)");
		return CORBA_FALSE;
	}
	switch (tc->kind) {
		case CORBA_tk_null:
		case CORBA_tk_void:
			return CORBA_TRUE;
		
		case CORBA_tk_string:
			return marshal_string(arg, buf);
		case CORBA_tk_short:
		case CORBA_tk_ushort:
			return marshal_short(arg, buf);
		case CORBA_tk_boolean:
			return marshal_boolean(arg, buf);
		case CORBA_tk_long:
		case CORBA_tk_ulong:
			return marshal_long(arg, buf);
		case CORBA_tk_char:
			return marshal_char(arg, buf);
		case CORBA_tk_octet:
			return marshal_octet(arg, buf);
		case CORBA_tk_float:
			return marshal_float(arg, buf);
		case CORBA_tk_double:
			return marshal_double(arg, buf);
		case CORBA_tk_longlong:
		case CORBA_tk_ulonglong:
			return marshal_longlong(arg, buf);

		case CORBA_tk_struct:
			return marshal_struct(arg, buf, tc);
		case CORBA_tk_array:
			return marshal_array(arg, buf, tc);
		case CORBA_tk_sequence:
			return marshal_sequence(arg, buf, tc);
		case CORBA_tk_enum:
			return marshal_enum(arg, buf, tc);
		case CORBA_tk_union:
			return marshal_union(arg, buf, tc);
		case CORBA_tk_alias:
			return marshal_arg(arg, buf, tc->subtypes[0]);
		case CORBA_tk_any:
			return marshal_any(arg, buf);

		case CORBA_tk_wchar:
		case CORBA_tk_wstring:
		case CORBA_tk_Principal:
			g_warning("Can't marshal unsupported typecode: %s", tc->kind);
			return CORBA_FALSE;
	}
	// Is this an object reference we know about?
	glue = (CORBA_PyObject_Glue *)g_hash_table_lookup(object_glue, tc->repo_id);
	if (glue)
		return marshal_object(arg, buf);
	else
		PyErr_Format(PyExc_TypeError, "Failed to marshal: %s (%s)",
		                tc->name, tc->repo_id);
	return CORBA_FALSE;
}

	
