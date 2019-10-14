#include <Python.h>
#include <orb/interface_repository.h>
#include "CORBAmodule.h"
#include "types.h"

#define RECV_BUFFER_LEFT(buf) \
 (((guchar *)buf->message_body + GIOP_MESSAGE_BUFFER(buf)->message_header.message_size) - (guchar *)buf->cur)

#define advance_buf(b, l) { char *z = (char *)b->cur; z += l; b->cur = z; }

CORBA_boolean buf_getn (GIOPRecvBuffer *buf, void *dest, size_t n)
{

	d_message(4, "buf_getn: entered");

	if (RECV_BUFFER_LEFT(buf) < n) {
		g_warning("incomplete message received");
		return CORBA_FALSE;
	}
	buf->cur = ALIGN_ADDRESS(buf->cur, n);
	buf->decoder(dest, (buf->cur), n);
	buf->cur = ((guchar *)buf->cur) + n;

	return CORBA_TRUE;
}

PyObject *demarshal_short(GIOPRecvBuffer *buf)
{
	CORBA_short v;

	d_message(4, "demarshal_short: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("h", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_boolean(GIOPRecvBuffer *buf)
{
	CORBA_octet v;

	d_message(4, "demarshal_boolean: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("h", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_string(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	char *s;
	CORBA_unsigned_long len;
	PyObject *o;

	d_message(4, "demarshal_string: entered");

	if (!buf_getn(buf, &len, sizeof(len)))
		return NULL;
	if (tc->length != 0 && len - 1 > tc->length) {
		raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                          CORBA_COMPLETED_YES,
		                       "String received is too long!");
		return NULL;
	}
	s = g_new(char, len);
	memmove(s, buf->cur, len);
	advance_buf(buf, len);

	s[len - 1] = '\0';
	o = PyString_FromString(s);
	g_free(s);
	return o;
}

void demarshal_exception(GIOPRecvBuffer *buf, CORBA_TypeCode tc,
                         CORBA_exception_type type,
                         CORBA_OperationDescription *opd)
{
	CORBA_unsigned_long len, minor, status, i;
	char *repo_id;
	PyObject *data = NULL, *o_str, *cl;

	d_message(4, "demarshal_exception: entered");

	g_return_if_fail (type != CORBA_NO_EXCEPTION );
	if (!buf_getn(buf, &len, sizeof(len)))
		goto skip;
	if (*((char *)buf->cur + len - 1) != '\0')
		goto skip;

	repo_id = (char *)buf->cur;
	advance_buf(buf, len);
	if (type == CORBA_USER_EXCEPTION) {
		if (opd)
			for (i = 0; i < opd->exceptions._length; i++) 
				if (!strcmp(opd->exceptions._buffer[i].id, repo_id)) {
					tc = opd->exceptions._buffer[i].type;
					break;
				}
		if (!tc)
			return raise_system_exception("IDL:omg.org/CORBA/UNKNOWN:1.0", 0, 
			                       CORBA_COMPLETED_MAYBE, "Unkown exception: %s",
			                       repo_id);
	}
	if (!tc)
		return raise_system_exception(repo_id, 0, CORBA_COMPLETED_MAYBE, NULL);
	// Demarshal the exception values
	cl = (PyObject *)g_hash_table_lookup(exceptions, repo_id);
	data = PyInstance_New(cl, NULL, NULL);

	for (i = 0; i < tc->sub_parts; i++) {
		PyObject *o = demarshal_arg(buf, tc->subtypes[i]);
		PyObject_SetAttrString(data, (char *)tc->subnames[i], o);
	}
	// Raise the exception
skip:
	raise_user_exception(repo_id, data);
}

PyObject *demarshal_object(GIOPRecvBuffer *buf)
{
	CORBA_Object obj = ORBit_demarshal_object(buf, orb_instance->obj);

	d_message(4, "demarshal_object: entered");

	if (!check_corba_ex(&orb_instance->ev))
		return NULL;
	return (PyObject *)CORBA_Object_to_PyObject(obj);
}

PyObject *demarshal_struct(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	PyObject *cl = (PyObject *)g_hash_table_lookup(object_glue, tc->repo_id),
	         *data;
	CORBA_unsigned_long i;

	d_message(4, "demarshal_struct: entered");

	// FIXME: throw an exception
	if (!cl) {
		g_warning("Asked to demarshal unknown struct (%s)", tc->repo_id);
		return NULL;
	}
	data = PyInstance_New(cl, NULL, NULL);
	for (i = 0; i < tc->sub_parts; i++) {
		PyObject *o = demarshal_arg(buf, tc->subtypes[i]);
		PyObject_SetAttrString(data, (char *)tc->subnames[i], o);
	}
	return data;
}
	
PyObject *demarshal_array(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	PyObject *tuple = PyTuple_New(tc->length);
	CORBA_unsigned_long i;

	d_message(4, "demarshal_array: entered");

	for (i = 0; i < tc->length; i++) {
		PyObject *o = demarshal_arg(buf, tc->subtypes[0]);
		if (!o) return NULL;
		PyTuple_SetItem(tuple, i, o);
	}
	return tuple;
}

PyObject *demarshal_sequence(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	CORBA_unsigned_long len, i;
	PyObject *tuple;

	d_message(4, "demarshal_sequence: entered");

	if (!buf_getn(buf, &len, sizeof(len))) // FIXME: raise exception
		return NULL;
	// TODO: handle sequences of chars/octets as strings
	d_message(3, "demarshal_sequence: length = %d", len);
	tuple = PyTuple_New(len);
	for (i = 0; i < len; i++) {
		PyObject *o = demarshal_arg(buf, tc->subtypes[0]);
		if (!o) return NULL;
		PyTuple_SetItem(tuple, i, o);
	}
	return tuple;
}

PyObject *demarshal_enum(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	CORBA_unsigned_long v;

	d_message(4, "demarshal_enum: entered");

	if (!buf_getn(buf, &v, sizeof(v)))
		return NULL;
	return PyInt_FromLong(v);
}

PyObject *demarshal_union(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	PyObject *v, *d, *inst, *cl;
	CORBA_long arm;

	d_message(4, "demarshal_union: entered");

	cl = (PyObject *)g_hash_table_lookup(object_glue, tc->repo_id);
	if (!cl) {
		g_warning("Unregistered union type: %s", tc->repo_id);
		return NULL;
	}
	d = demarshal_arg(buf, tc->discriminator);
	if (!d) { // FIXME: raise exception
		g_warning("d is NULL");
		return NULL;
	}
	arm = find_union_arm(tc, d);
	if (arm >= 0) {
		PyObject *tuple;
		v = demarshal_arg(buf, tc->subtypes[arm]);
		if (!v)
			return NULL;
		tuple = Py_BuildValue("OO", d, v);	
		inst = PyInstance_New(cl, tuple, NULL);
		return inst;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *demarshal_long(GIOPRecvBuffer *buf)
{
	CORBA_long v;

	d_message(4, "demarshal_long: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("l", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_char(GIOPRecvBuffer *buf)
{
	CORBA_char v;

	d_message(4, "demarshal_char: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("c", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_octet(GIOPRecvBuffer *buf)
{
	CORBA_octet v;

	d_message(4, "demarshal_octet: entered");

	if (buf_getn(buf, &v, sizeof(CORBA_char)))
		return Py_BuildValue("b", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_float(GIOPRecvBuffer *buf)
{
	CORBA_float v;

	d_message(4, "demarshal_float: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("f", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_double(GIOPRecvBuffer *buf)
{
	CORBA_double v;

	d_message(4, "demarshal_double: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("d", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_longlong(GIOPRecvBuffer *buf)
{
	CORBA_long_long v;

	d_message(4, "demarshal_longlong: entered");

	if (buf_getn(buf, &v, sizeof(v)))
		return Py_BuildValue("l", v);
	raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
	                       CORBA_COMPLETED_YES, NULL);
	return NULL;
}

PyObject *demarshal_any(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	CORBA_TypeCode res_tc;
	CORBA_TypeCode_PyObject *to;
	CORBA_any_PyObject *ao;
	PyObject *val;

	d_message(4, "demarshal_any: entered");

	ORBit_decode_CORBA_TypeCode(&res_tc, buf);

	val = demarshal_arg(buf, res_tc);
	if (!val) {
		raise_system_exception("IDL:omg.org/CORBA/MARSHAL:1.0", 0,
		                       CORBA_COMPLETED_YES, NULL);
		return NULL;
	}

	CORBA_Object_duplicate((CORBA_Object)res_tc, NULL);

	to = PyObject_NEW(CORBA_TypeCode_PyObject, &CORBA_TypeCode_PyObject_Type);
   ao = PyObject_NEW(CORBA_any_PyObject, &CORBA_any_PyObject_Type);
	to->tc = res_tc;
	to->repo_id = find_repo_id_from_typecode(res_tc);
	if (to->repo_id)
		to->repo_id = g_strdup(to->repo_id);
	ao->tc_object = (PyObject *)to;
	ao->value = val;
	return (PyObject *)ao;
}

PyObject *demarshal_arg(GIOPRecvBuffer *buf, CORBA_TypeCode tc)
{
	d_message(2, "demarshal_arg: tc->kind = %d, tc->name = %s", 
	          tc->kind, tc->name);
	switch (tc->kind) {
		case CORBA_tk_null:
			Py_INCREF(Py_None);
			return Py_None;
		case CORBA_tk_void:
			return NULL;
		case CORBA_tk_string:
			return demarshal_string(buf, tc);
		case CORBA_tk_short:
		case CORBA_tk_ushort:
			return demarshal_short(buf);
		case CORBA_tk_boolean:
			return demarshal_boolean(buf);
		case CORBA_tk_long:
		case CORBA_tk_ulong:
			return demarshal_long(buf);
		case CORBA_tk_char:
			return demarshal_char(buf);
		case CORBA_tk_float:
			return demarshal_float(buf);
		case CORBA_tk_double:
			return demarshal_double(buf);
		case CORBA_tk_longlong:
		case CORBA_tk_ulonglong:
			return demarshal_longlong(buf);

		case CORBA_tk_struct:
			return demarshal_struct(buf, tc);
		case CORBA_tk_array:
			return demarshal_array(buf, tc);
		case CORBA_tk_sequence:
			return demarshal_sequence(buf, tc);
		case CORBA_tk_enum:
			return demarshal_enum(buf, tc);
		case CORBA_tk_union:
			return demarshal_union(buf, tc);
		case CORBA_tk_alias:
			return demarshal_arg(buf, tc->subtypes[0]);
		case CORBA_tk_any:
			return demarshal_any(buf, tc);
		case CORBA_tk_objref:
			return demarshal_object(buf);

		case CORBA_tk_wchar:
		case CORBA_tk_wstring:
		case CORBA_tk_Principal:
		default:
			g_warning("Can't demarshal unsupported typecode: %s", tc->kind);
			return NULL;

	}
}
