#include "CORBAmodule.h"
#include <libIDL/IDL.h>

// for get_union_typecode
#define C_M(kind, type, value) \
	case kind: { \
		type *val; \
		val = g_new(type, 1); \
		*val = value; \
		res->sublabels[i]._value = val; \
		break; \
	}

typedef struct {
	GSList *ops, *attrs;
} InterfaceData;

GHashTable *type_codes;

CORBA_TypeCode get_typecode(IDL_tree);
CORBA_TypeCode get_ident_typecode(IDL_tree tree);
CORBA_TypeCode get_interface_typecode(IDL_tree tree);
CORBA_TypeCode get_union_typecode(IDL_tree tree);

GPtrArray *get_hierarchy(IDL_tree tree, PyObject *top, IDL_tree ident)
{
	GPtrArray *a = g_ptr_array_new();
	char *full = g_strdup(IDL_ns_ident_to_qstring(ident, ".", 0)),
	     *p = full, *q;
	PyObject *dict = top;

	if (!ident)
		ident = IDL_INTERFACE(tree).ident;
	g_ptr_array_add(a, dict);
	while ((q = strstr(p, "."))) {
		*q = 0;
		dict = PyObject_GetAttrString(dict, p);
		if (!dict) g_error("Put a more useful error here.");
		g_ptr_array_add(a, dict);
		p = q + 1;
	}
	// Try to lookup p (last term)
	dict = PyObject_GetAttrString(dict, p);
	if (dict)
		g_ptr_array_add(a, dict);
	else PyErr_Clear();
	g_free(full);
	return a;
}

void add_object_to_hierarchy(IDL_tree tree, PyObject *top, PyObject *o,
                             IDL_tree ident)
{
	char *p;
	GPtrArray *a;
	PyObject *instance;
	if (!ident)
		ident = IDL_INTERFACE(tree).ident;

	p = IDL_IDENT(ident).str;
	a = get_hierarchy(tree, top, ident);
	instance = (PyObject *)a->pdata[a->len - 1];
	PyObject_SetAttrString(instance, p, o);
	g_ptr_array_free(a, FALSE);
}


/*
 Make CORBA.module.interface
 XXX: need to make CORBA.POA.module.interface?
      i.e. servant = CORBA.POA.module.interface(server_object)
*/

void construct_interface(IDL_tree tree, InterfaceData *idata)
{
	IDL_tree ident = IDL_INTERFACE(tree).ident;
	GSList *t;
	CORBA_unsigned_long len, i;
   IDL_tree inheritance_spec;
	PyTypeObject *type;
	CORBA_PyObject_Glue *glue;
	PyMethodDef *def;
	PyObject *repo_id_object, *c_func;

	CORBA_InterfaceDef_FullInterfaceDescription *desc;
	desc = g_new0(CORBA_InterfaceDef_FullInterfaceDescription, 1);
	desc->name = g_strdup(IDL_IDENT(ident).str);
	desc->id = g_strdup(IDL_IDENT_REPO_ID(ident));
	desc->operations._length = g_slist_length(idata->ops);
	desc->operations._buffer =
	   CORBA_sequence_CORBA_OperationDescription_allocbuf(
	      desc->operations._length);

	// Define all the methods
	t = idata->ops;
	for (i = 0; i < desc->operations._length; i++) {
		CORBA_OperationDescription *d = (CORBA_OperationDescription *)t->data;
		// Set up the ops for interface desc
		desc->operations._buffer[i] = *d;
		g_free(t->data);
		t = t->next;
	}
	g_slist_free(idata->ops);
	// Done with the methods.

	// Now setup all the attributes.
	len = (CORBA_unsigned_long)g_slist_length(idata->attrs);
	desc->attributes._length = len;
	desc->attributes._buffer =
	   CORBA_sequence_CORBA_AttributeDescription_allocbuf(len);
	desc->attributes._release = TRUE;

	t = idata->attrs;
	for (i = 0; i < desc->attributes._length; i++) {
		desc->attributes._buffer[i] = *(CORBA_AttributeDescription *)t->data;
		g_free(t->data);
		t = t->next;
	}
	g_slist_free(idata->attrs);

	// Done with attributes.
	// Setup base interfaces
   inheritance_spec = IDL_INTERFACE (tree).inheritance_spec;
	len = IDL_list_length(inheritance_spec);
	desc->base_interfaces._length = len;
	desc->base_interfaces._buffer = 
	   CORBA_sequence_CORBA_RepositoryId_allocbuf(len);
	desc->base_interfaces._release = TRUE;
	for (i = 0; i < len; i++) {
		IDL_tree ident = IDL_LIST(inheritance_spec).data;
		desc->base_interfaces._buffer[i] = IDL_IDENT_REPO_ID(ident);
		inheritance_spec = IDL_LIST(inheritance_spec).next;
	}
	
   // Setup the python type for this object (client side).	
	type = g_new0(PyTypeObject, 1);
	type->ob_refcnt = 1;
	type->tp_name = IDL_ns_ident_to_qstring(ident, ".", 0);
	type->tp_basicsize = sizeof(CORBA_PyObject);
   type->tp_dealloc = (destructor)CORBA_PyObject__dealloc;
   type->tp_getattr = (getattrfunc)CORBA_PyObject__getattr;
   type->tp_setattr = (setattrfunc)CORBA_PyObject__setattr;

	// glue this type to the repo id
	glue = g_new(CORBA_PyObject_Glue, 1);
	glue->type = type;
	glue->desc = desc;

	glue->generic_method = g_new0(PyMethodDef, 1);
	glue->generic_method->ml_name = g_strdup("__method_stub__");
	glue->generic_method->ml_meth = (PyCFunction)stub_func;
	glue->generic_method->ml_flags = 1;

	g_hash_table_insert(object_glue, desc->id, glue);

	// Now make the POA for this object
	def = g_new(PyMethodDef, 1);
	def->ml_name = g_strdup(IDL_ns_ident_to_qstring(ident, ".", 1));
	def->ml_meth = (PyCFunction)new_poa_servant;
	def->ml_flags = 1;
	repo_id_object = PyString_FromString(desc->id);
	c_func = PyCFunction_New(def, repo_id_object);
	add_object_to_hierarchy(tree, poa_instances, c_func, NULL);

	// Setup the python type for the servant
	type = g_new0(PyTypeObject, 1);
	type->ob_refcnt = 1;
	type->tp_name = g_strconcat("Servant.", glue->type->tp_name, NULL);
	type->tp_basicsize = sizeof(PyTypeObject);
	type->tp_dealloc = (destructor)Servant_PyObject__dealloc;
	type->tp_getattr = (getattrfunc)Servant_PyObject__getattr;
	type->tp_setattr = (setattrfunc)Servant_PyObject__setattr;
	glue->servant_type = type;
}

/////////

static CORBA_OperationDescription *do_operation(IDL_tree tree)
{
	gboolean f_oneway = IDL_OP_DCL (tree).f_oneway;
	IDL_tree op_type_spec = IDL_OP_DCL (tree).op_type_spec;
	IDL_tree ident = IDL_OP_DCL (tree).ident;
	IDL_tree dcls = IDL_OP_DCL (tree).parameter_dcls;
	IDL_tree raises_expr = IDL_OP_DCL (tree).raises_expr;
	CORBA_unsigned_long i;

	CORBA_OperationDescription *opd = g_new0(CORBA_OperationDescription, 1);
	opd->name = g_strdup(IDL_IDENT(ident).str);
	opd->id = g_strdup(IDL_IDENT_REPO_ID(ident));
	d_message(3, "do_operation: %s (%s)", opd->name, opd->id);

	// Set the return type for this function
	if (op_type_spec)
		opd->result = get_typecode(op_type_spec);
	else
		opd->result = dupe_tc(TC_void);

	opd->mode = f_oneway ? CORBA_OP_ONEWAY : CORBA_OP_NORMAL;

	opd->parameters._length = IDL_list_length(dcls);
	opd->parameters._buffer = 
		CORBA_sequence_CORBA_ParameterDescription_allocbuf(IDL_list_length(dcls));


	for (i = 0; i < opd->parameters._length; i++) {
		CORBA_ParameterDescription *par_desc = &opd->parameters._buffer[i];
		IDL_tree dcl = IDL_LIST(dcls).data;
		enum IDL_param_attr attr;

		par_desc->name = IDL_IDENT(IDL_PARAM_DCL(dcl).simple_declarator).str;
		par_desc->type = get_typecode(IDL_PARAM_DCL(dcl).param_type_spec);
		attr = IDL_PARAM_DCL(dcl).attr;
		if (attr == IDL_PARAM_IN) par_desc->mode = CORBA_PARAM_IN;
		else if (attr == IDL_PARAM_OUT) par_desc->mode = CORBA_PARAM_OUT;
		else if (attr == IDL_PARAM_INOUT) par_desc->mode = CORBA_PARAM_INOUT;
		par_desc->type_def = CORBA_OBJECT_NIL;
		
		dcls = IDL_LIST(dcls).next;
		d_message(3, "do_operation: parameter %s (%s)", par_desc->name,
		          par_desc->type->name);
	}

	// handle exception stuff
	opd->exceptions._length = IDL_list_length(raises_expr);
	opd->exceptions._buffer =
            CORBA_sequence_CORBA_ExceptionDescription_allocbuf(
	                      opd->exceptions._length);
	opd->exceptions._release = TRUE;
	for (i = 0; i < opd->exceptions._length; i++) {
		CORBA_ExceptionDescription *exd = &opd->exceptions._buffer[i];
		IDL_tree ref = IDL_LIST(raises_expr).data;
		exd->type = get_ident_typecode(ref);
		if (exd->type->kind != CORBA_tk_except)
			g_error("Raises non-exception");
		exd->id = g_strdup(exd->type->repo_id);
		exd->name = exd->defined_in = exd->version = NULL;
		raises_expr = IDL_LIST(raises_expr).next;
	}
	// done
	return opd;	
}

GSList *do_attribute(IDL_tree tree)
{
	CORBA_AttributeDescription *attr_desc;
	gboolean f_readonly = IDL_ATTR_DCL(tree).f_readonly;
	GSList *result = NULL;
	IDL_tree pts = IDL_ATTR_DCL(tree).param_type_spec;
	IDL_tree simple_decls = IDL_ATTR_DCL(tree).simple_declarations;

	while (simple_decls) {
		IDL_tree ident = IDL_LIST(simple_decls).data;
		attr_desc = g_new0(CORBA_AttributeDescription, 1);
		attr_desc->name = g_strdup(IDL_IDENT(ident).str);
		attr_desc->id = g_strdup(IDL_IDENT_REPO_ID(ident));
		attr_desc->type = get_typecode(pts);
		attr_desc->mode = f_readonly ? CORBA_ATTR_READONLY : CORBA_ATTR_NORMAL;
		result = g_slist_prepend(result, attr_desc);

		simple_decls = IDL_LIST(simple_decls).next;
	}
	return result;
}

void do_exception(IDL_tree tree)
{
	PyObject *except;
	IDL_tree ident = IDL_TYPE_ENUM(tree).ident;
	char *full_name = IDL_ns_ident_to_qstring(ident, ".", 0), *repo_id;

	if (!strstr(full_name, "."))
		full_name = g_strconcat(".", full_name, NULL);
	except = PyErr_NewException(full_name, UserExcept, NULL);
	if (except) {
	   PyMethodDef *def = g_new(PyMethodDef, 1);
		PyObject *func, *meth;
	   def->ml_name = g_strdup("__init__");
	   def->ml_meth = (PyCFunction)UserExcept_PyClass__init;
	   def->ml_flags = METH_VARARGS | METH_KEYWORDS;
	   func = PyCFunction_New(def, except);
		meth = PyMethod_New(func, NULL, except);
		PyObject_SetAttrString(except, "__init__", meth);
	}

	// Hopefully this shouldn't happen!
	if (PyErr_Occurred())  {
		g_warning("Error in except");
		PyErr_Print();
		g_error("bail");
	}
	repo_id = IDL_IDENT_REPO_ID(ident);
	g_hash_table_insert(exceptions, repo_id, except);
	PyObject_SetAttrString(except, "__repo_id", PyString_FromString(repo_id));
	add_object_to_hierarchy(tree, main_module, except, NULL);
}

// TODO: give do_struct and do_exception a common backend
void do_struct(IDL_tree tree)
{
	IDL_tree ident = IDL_TYPE_ENUM(tree).ident;
	char *full_name = IDL_ns_ident_to_qstring(ident, ".", 0),
	     *p = IDL_IDENT(ident).str, *repo_id;
	PyObject *cl_name, *cl;

	if (!strstr(full_name, ".")) // FIXME: this will leak memory
		full_name = g_strconcat(".", full_name, NULL);

	cl_name = PyString_FromString(full_name);
	cl = PyClass_New(NULL, PyDict_New(), cl_name);
	// XXX: Find out why I get:
	// TypeError: attribute-less object (assign or del)
	PyErr_Clear();

	if (cl) {
	   PyMethodDef *def = g_new(PyMethodDef, 1);
		PyObject *func, *meth;
	   def->ml_name = g_strdup("__init__");
	   def->ml_meth = (PyCFunction)UserExcept_PyClass__init;
	   def->ml_flags = METH_VARARGS | METH_KEYWORDS;
	   func = PyCFunction_New(def, cl);
		meth = PyMethod_New(func, NULL, cl);
		PyObject_SetAttrString(cl, "__init__", meth);
	}
	// Hopefully this shouldn't happen!
	if (PyErr_Occurred())  {
		PyErr_Print();
		return;
	}
	repo_id = IDL_IDENT_REPO_ID(ident);
	g_hash_table_insert(object_glue, repo_id, cl);
	PyObject_SetAttrString(cl, "__repo_id", PyString_FromString(repo_id));
	add_object_to_hierarchy(tree, main_module, cl, NULL);
}

void do_enum(IDL_tree tree)
{
	IDL_tree ident = IDL_TYPE_ENUM(tree).ident;
	IDL_tree elist = IDL_TYPE_ENUM(tree).enumerator_list;
	PyObject *tuple;

	// Figure out how many members
	CORBA_unsigned_long i = 0;
	IDL_tree tmp = elist;
	while (tmp) {
		i++;
		tmp = IDL_LIST(tmp).next;
	}
	tuple = PyTuple_New(i);

	tmp = elist;
	i = 0;
	while (tmp) {
		IDL_tree ident = IDL_LIST(tmp).data;
		PyObject *v = PyInt_FromLong(i);
		add_object_to_hierarchy(tmp, main_module, v, NULL);
		PyTuple_SetItem(tuple, i++, PyString_FromString(IDL_IDENT(ident).str));
		tmp = IDL_LIST(tmp).next;
	}
	add_object_to_hierarchy(IDL_NODE_UP(elist), main_module, tuple, NULL);
}
	
void do_union(IDL_tree tree)
{
	IDL_tree ident = IDL_TYPE_ENUM(tree).ident;
	char *full_name = IDL_ns_ident_to_qstring(ident, ".", 0),
	     *p = IDL_IDENT(ident).str, *repo_id;
	PyObject *cl_name, *cl;

	if (!strstr(full_name, ".")) // FIXME: this will leak memory
		full_name = g_strconcat(".", full_name, NULL);

	cl_name = PyString_FromString(full_name);
	cl = PyClass_New(NULL, PyDict_New(), cl_name);
	// XXX: Find out why I get:
	// TypeError: attribute-less object (assign or del)
	PyErr_Clear();

	if (cl) {
	   PyMethodDef *def = g_new(PyMethodDef, 1);
		PyObject *func, *meth;
	   def->ml_name = g_strdup("__init__");
	   def->ml_meth = (PyCFunction)Union_PyClass__init;
	   def->ml_flags = METH_VARARGS;
	   func = PyCFunction_New(def, cl);
		meth = PyMethod_New(func, NULL, cl);
		PyObject_SetAttrString(cl, "__init__", meth);
	}
	// Hopefully this shouldn't happen!
	if (PyErr_Occurred())  {
		PyErr_Print();
		return;
	}
	repo_id = IDL_IDENT_REPO_ID(ident);
	g_hash_table_insert(object_glue, repo_id, cl);
	PyObject_SetAttrString(cl, "__repo_id", PyString_FromString(repo_id));
	add_object_to_hierarchy(tree, main_module, cl, NULL);
}

void do_const(IDL_tree tree)
{
	CORBA_TypeCode tc = get_typecode(IDL_CONST_DCL(tree).const_type);

	IDL_tree value = IDL_CONST_DCL(tree).const_exp;
	IDL_tree ident = IDL_CONST_DCL(tree).ident;
	PyObject *o = NULL;
	switch (tc->kind) {
		case CORBA_tk_long:
		case CORBA_tk_ulong:
		case CORBA_tk_short:
		case CORBA_tk_ushort:
		case CORBA_tk_ulonglong:
			o = PyInt_FromLong(IDL_INTEGER(value).value);
			break;
		case CORBA_tk_boolean:
			o = PyInt_FromLong(IDL_BOOLEAN(value).value);
			break;
		case CORBA_tk_char:
			o = Py_BuildValue("c", IDL_CHAR(value).value);
			break;
		case CORBA_tk_float:
		case CORBA_tk_double:
			o = PyFloat_FromDouble(IDL_FLOAT(value).value);
			break;
		case CORBA_tk_string:
			o = PyString_FromString(IDL_STRING(value).value);
			break;

		case CORBA_tk_longdouble:
		case CORBA_tk_wchar:
		case CORBA_tk_fixed:
		case CORBA_tk_wstring:
		default:
			g_warning("Unsupported constant type: %d", tc->kind);
			break;
	}

	if (o)
		add_object_to_hierarchy(tree, main_module, o, IDL_CONST_DCL(tree).ident);
	CORBA_Object_release((CORBA_Object)tc, NULL);
}

gboolean tree_pre_func(IDL_tree_func_data *tfd, gpointer user_data)
{
	IDL_tree tree = tfd->tree;
	switch(IDL_NODE_TYPE(tree)) {
		case IDLN_LIST:
			return TRUE;

		case IDLN_OP_DCL:
		{
			InterfaceData *idata = (InterfaceData *)tfd->up->up->data;
			idata->ops = g_slist_prepend(idata->ops, do_operation(tfd->tree));
			return FALSE;
		}
		case IDLN_ATTR_DCL:
		{
			InterfaceData *idata = (InterfaceData *)tfd->up->up->data;
			idata->attrs = g_slist_concat(do_attribute(tfd->tree), idata->attrs);
			return FALSE;
		}

		case IDLN_INTERFACE:
		{
			InterfaceData *idata = g_new0(InterfaceData, 1);
			IDL_tree ident;
			char *p;
			PyObject *class_name, *new_class, *o;

			tfd->data = idata;
			ident = IDL_INTERFACE(tree).ident;
			p = IDL_IDENT(ident).str;
			IDL_ns_ident_to_qstring(ident, ".", 0);

			// Now create an instance for the client
			// FIXME: check to see if this interface already exists
			class_name = PyString_FromString("NewClass");
			new_class = PyClass_New(NULL, PyDict_New(), class_name);
			o = PyInstance_New(new_class, NULL, NULL);
			add_object_to_hierarchy(tree, main_module, new_class, NULL);

			// store the typecode for this interface
			CORBA_Object_release((CORBA_Object)get_interface_typecode(tree), NULL);
		
			return TRUE;
		}

		case IDLN_MODULE:
		{
			IDL_tree ident = IDL_MODULE(tree).ident;
			char *p = IDL_IDENT(ident).str, *repo_id = IDL_IDENT(ident).repo_id;
			PyObject *class_name, *new_class, *o;

			// FIXME: Need something more appropriate here.
			// FIXME: check to see if this module already exists
			class_name = PyString_FromString("NewClass");
			new_class = PyClass_New(NULL, PyDict_New(), class_name);
			o = PyInstance_New(new_class, NULL, NULL);
			add_object_to_hierarchy(tree, poa_instances, o, NULL);

			// Now create an instance for the client
			o = PyInstance_New(new_class, NULL, NULL);
			add_object_to_hierarchy(tree, main_module, o, NULL);

			return TRUE;
		}

		case IDLN_EXCEPT_DCL:
			do_exception(tfd->tree);
			return FALSE;

		case IDLN_TYPE_STRUCT:
			do_struct(tfd->tree);
			return FALSE;

		case IDLN_TYPE_ENUM:
			do_enum(tfd->tree);
			return FALSE;

		case IDLN_TYPE_UNION:
			do_union(tfd->tree);
			CORBA_Object_release((CORBA_Object)get_union_typecode(tree), NULL);
			return FALSE;

		case IDLN_CONST_DCL:
			do_const(tfd->tree);
			return FALSE;

		default:
			return FALSE;
			
	}
}

gboolean tree_post_func(IDL_tree_func_data *tfd, gpointer user_data)
{
	IDL_tree tree = tfd->tree;
	switch(IDL_NODE_TYPE(tree)) {
		case IDLN_INTERFACE:
		{
			construct_interface(tree, (InterfaceData *)tfd->data);
			break;
		}
			
	}
	return TRUE;
}

CORBA_boolean parse(char *file, char *params)
{
	IDL_tree tree;
	IDL_ns ns;
	int ret;

	ret = IDL_parse_filename(file, params, NULL, &tree, &ns, 
	                         IDLF_TYPECODES | IDLF_CODEFRAGS, IDL_WARNING1);

	if (ret == IDL_ERROR) {
		raise_system_exception("IDL:omg.org/CORBA/UNKNOWN:1.0", 0,
		                       CORBA_COMPLETED_NO, "Error parsing IDL");
		return CORBA_FALSE;
	}
	else if (ret < 0) {
		raise_system_exception("IDL:omg.org/CORBA/UNKNOWN:1.0", 0,
		                       CORBA_COMPLETED_NO, "Error parsing IDL: %s",
		                       g_strerror(errno));
		return CORBA_FALSE;
	}

	IDL_tree_walk(tree, NULL, tree_pre_func, tree_post_func, NULL);
	IDL_tree_free(tree);
	IDL_ns_free(ns);
	return CORBA_TRUE;
}	


////////////////////////////////////////////////////////////////////////
// Typecode routines, mostly taken from CORBA:ORBit

CORBA_TypeCode find_typecode(char *id)
{
	if (type_codes) {
		CORBA_TypeCode r;
		r = (CORBA_TypeCode)g_hash_table_lookup(type_codes, id);
		if (r)
			return dupe_tc(r);
	}
	return NULL;
}

void store_typecode(char *id, CORBA_TypeCode tc)
{
	g_hash_table_insert(type_codes, g_strdup(id), dupe_tc(tc));
}

void _find_repo_id_func(gpointer key, gpointer value, gpointer id)
{
	typedef struct {
		char *id;
		CORBA_TypeCode tc;
	} __tmp;
	__tmp *_tmp = (__tmp *)id;
	CORBA_TypeCode tc = (CORBA_TypeCode)value;
	if (tc->kind == _tmp->tc->kind)
		_tmp->id = (char *)key;
}

char *find_repo_id_from_typecode(CORBA_TypeCode tc)
{
	struct {
		char *id;
		CORBA_TypeCode tc;
	} _tmp = { NULL, tc };
		
	g_hash_table_foreach(type_codes, _find_repo_id_func, &_tmp);
	return _tmp.id;
}
	

void init_typecodes()
{
	type_codes = g_hash_table_new(g_str_hash, g_str_equal);
	store_typecode("IDL:CORBA/Short:1.0", dupe_tc(TC_CORBA_short));
	store_typecode("IDL:CORBA/UShort:1.0", dupe_tc(TC_CORBA_ushort));
	store_typecode("IDL:CORBA/Long:1.0", dupe_tc(TC_CORBA_long));
	store_typecode("IDL:CORBA/ULong:1.0", dupe_tc(TC_CORBA_ulong));
	store_typecode("IDL:CORBA/LongLong:1.0", dupe_tc(TC_CORBA_longlong));
	store_typecode("IDL:CORBA/ULongLong:1.0", dupe_tc(TC_CORBA_ulonglong));
	store_typecode("IDL:CORBA/Float:1.0", dupe_tc(TC_CORBA_float));
	store_typecode("IDL:CORBA/Double:1.0", dupe_tc(TC_CORBA_double));
	store_typecode("IDL:CORBA/LongDouble:1.0", dupe_tc(TC_CORBA_longdouble));
	store_typecode("IDL:CORBA/Boolean:1.0", dupe_tc(TC_CORBA_boolean));
	store_typecode("IDL:CORBA/Char:1.0", dupe_tc(TC_CORBA_char));
	store_typecode("IDL:CORBA/WChar:1.0", dupe_tc(TC_CORBA_wchar));
	store_typecode("IDL:CORBA/Octet:1.0", dupe_tc(TC_CORBA_octet));
	store_typecode("IDL:CORBA/Any:1.0", dupe_tc(TC_CORBA_any));
	store_typecode("IDL:CORBA/TypeCode:1.0", dupe_tc(TC_CORBA_TypeCode));
	store_typecode("IDL:CORBA/Principal:1.0", dupe_tc(TC_CORBA_Principal));
	store_typecode("IDL:CORBA/Object:1.0", dupe_tc(TC_CORBA_Object));
	store_typecode("IDL:CORBA/String:1.0", dupe_tc(TC_CORBA_string));
	store_typecode("IDL:CORBA/WString:1.0", dupe_tc(TC_CORBA_wstring));
}

CORBA_TypeCode alloc_typecode()
{
	CORBA_TypeCode res = g_new0(struct CORBA_TypeCode_struct, 1);
	ORBit_pseudo_object_init((ORBit_PseudoObject)res, ORBIT_PSEUDO_TYPECODE,
	                         NULL);
	ORBit_RootObject_set_interface((ORBit_RootObject)res,
	            (ORBit_RootObject_Interface *)&ORBit_TypeCode_epv,
	            NULL);
	return dupe_tc(res);
}

CORBA_TypeCode get_integer_typecode(IDL_tree tree)
{
	gboolean f_signed = IDL_TYPE_INTEGER(tree).f_signed;
	enum IDL_integer_type f_type = IDL_TYPE_INTEGER(tree).f_type;

	if (f_signed) {
		switch(f_type) {
			case IDL_INTEGER_TYPE_SHORT:
				return dupe_tc(TC_CORBA_short);
			case IDL_INTEGER_TYPE_LONG:
				return dupe_tc(TC_CORBA_long);
			case IDL_INTEGER_TYPE_LONGLONG:
				return dupe_tc(TC_CORBA_longlong);
		}
	}
	else {
		switch(f_type) {
			case IDL_INTEGER_TYPE_SHORT:
				return dupe_tc(TC_CORBA_ushort);
			case IDL_INTEGER_TYPE_LONG:
				return dupe_tc(TC_CORBA_ulong);
			case IDL_INTEGER_TYPE_LONGLONG:
				return dupe_tc(TC_CORBA_ulonglong);
		}
	}
	g_assert_not_reached();
	return NULL;
}

CORBA_TypeCode get_string_typecode(IDL_tree tree)
{
	IDL_tree pic = IDL_TYPE_STRING(tree).positive_int_const;
	CORBA_TypeCode res = alloc_typecode();
	res->kind = CORBA_tk_string;

	if (pic)
		res->length = IDL_INTEGER(pic).value;
	else 
		res->length = 0;

	return res;
}

CORBA_TypeCode get_wstring_typecode(IDL_tree tree)
{
	IDL_tree pic = IDL_TYPE_WIDE_STRING(tree).positive_int_const;
	CORBA_TypeCode res = alloc_typecode();
	res->kind = CORBA_tk_wstring;

	if (pic)
		res->length = IDL_INTEGER(pic).value;
	else
		res->length = 0;

	return res;
}

CORBA_TypeCode get_declarator_typecode(IDL_tree tree, CORBA_TypeCode base_tc)
{
	if (IDL_NODE_TYPE(tree) == IDLN_TYPE_ARRAY) {
		IDL_tree size_list = IDL_TYPE_ARRAY(tree).size_list;
		IDL_tree tmp_list;
		CORBA_TypeCode res = dupe_tc(base_tc);
		CORBA_TypeCode child_tc;

		tmp_list = IDL_LIST(size_list)._tail;
		while (tmp_list) {
			IDL_tree size = IDL_LIST(tmp_list).data;
			child_tc = res;
			res = alloc_typecode();
			res->kind = CORBA_tk_array;
			res->length = IDL_INTEGER(size).value;
			res->sub_parts = 1;
			res->subtypes = g_new(CORBA_TypeCode, 1);
			res->subtypes[0] = child_tc;

			tmp_list = IDL_LIST(tmp_list).prev;
		}
		return res;
	}
	else if (IDL_NODE_TYPE(tree) == IDLN_IDENT)
		return dupe_tc(base_tc);
	else
		g_warning("get_declarator_typecode() needs ident or array.");
	return NULL;
}

gchar *get_declarator_name(IDL_tree tree)
{	
	if (IDL_NODE_TYPE(tree) == IDLN_TYPE_ARRAY) 
		return g_strdup(IDL_IDENT(IDL_TYPE_ARRAY(tree).ident).str);
	else if (IDL_NODE_TYPE(tree) == IDLN_IDENT)
		return g_strdup(IDL_IDENT(tree).str);
	else
		g_warning("get_declarator_name() needs ident or array.");
	return NULL;
}

CORBA_TypeCode get_ident_typecode(IDL_tree tree)
{
	IDL_tree parent;
	CORBA_TypeCode res;
	char *repo_id;

	repo_id = IDL_IDENT_REPO_ID(tree);
	res = find_typecode(repo_id);
	if (res)	
		return res;
	
	parent = IDL_NODE_UP(tree);
	switch (IDL_NODE_TYPE(parent)) {
		case IDLN_TYPE_ENUM:
		case IDLN_EXCEPT_DCL:
		case IDLN_INTERFACE:
		case IDLN_TYPE_STRUCT:
		case IDLN_TYPE_UNION:
			return get_typecode(parent);
		case IDLN_TYPE_ARRAY:
		{
			IDL_tree list;
			IDL_tree dcl;
			CORBA_TypeCode base_tc;

			g_assert(IDL_NODE_TYPE(IDL_NODE_UP(parent)) == IDLN_LIST);
			list = IDL_NODE_UP(parent);
			g_assert(IDL_NODE_TYPE(IDL_NODE_UP(list)) == IDLN_TYPE_DCL);
			dcl = IDL_NODE_UP(list);
			base_tc = get_typecode(IDL_TYPE_DCL(dcl).type_spec);
			res = get_declarator_typecode(parent, base_tc);
			CORBA_Object_release((CORBA_Object)base_tc, NULL);
			store_typecode(repo_id, res);
			return res;
		}
		case IDLN_LIST:
		{
			IDL_tree dcl;
			g_assert(IDL_NODE_TYPE(IDL_NODE_UP(parent)) == IDLN_TYPE_DCL);
			dcl = IDL_NODE_UP(parent);
			res = get_typecode(IDL_TYPE_DCL(dcl).type_spec);
			store_typecode(repo_id, res);
			return res;
		}
		default:
			g_warning("Reference to node type %s is invalid",
			          IDL_NODE_TYPE_NAME(parent));
	}
	g_assert_not_reached();
	return NULL;
}

CORBA_TypeCode get_exception_typecode(IDL_tree tree)
{
	IDL_tree ident = IDL_EXCEPT_DCL(tree).ident;
	IDL_tree members = IDL_EXCEPT_DCL(tree).members;
	IDL_tree tmp1, tmp2;
	char *repo_id = IDL_IDENT_REPO_ID(ident);
	CORBA_unsigned_long i = 0;

	CORBA_TypeCode res = find_typecode(repo_id);
	if (res)
		return res;
	res = alloc_typecode();
	res->kind = CORBA_tk_except;
	res->repo_id = g_strdup(repo_id);
	res->name = g_strdup(IDL_IDENT(ident).str);

	// Figure out how many members we have
	res->sub_parts = 0;
	tmp1 = members;
	while (tmp1) {
		IDL_tree member = IDL_LIST(tmp1).data;
		IDL_tree dcls = IDL_MEMBER(member).dcls;
		res->sub_parts += IDL_list_length(dcls);
		tmp1 = IDL_LIST(tmp1).next;
	}
	// Allocate memory for these members.
	res->subnames = g_new(const gchar *, res->sub_parts);
	res->subtypes = g_new(CORBA_TypeCode, res->sub_parts);
	// Now fill in the blanks
	tmp1 = members;
	while (tmp1) {
		IDL_tree member = IDL_LIST(tmp1).data;
		IDL_tree type_spec = IDL_MEMBER(member).type_spec;
		IDL_tree dcls = IDL_MEMBER(member).dcls;
		CORBA_TypeCode base_tc = get_typecode(type_spec);

		IDL_tree tmp2 = dcls;
		while (tmp2) {
			IDL_tree dcl = IDL_LIST(tmp2).data;
			res->subnames[i] = get_declarator_name(dcl);
			res->subtypes[i++] = get_declarator_typecode(dcl, base_tc);
			tmp2 = IDL_LIST(tmp2).next;
		}
		CORBA_Object_release((CORBA_Object)base_tc, NULL);
		tmp1 = IDL_LIST(tmp1).next;
	}
	store_typecode(repo_id, res);
	return res;
}

CORBA_TypeCode get_interface_typecode(IDL_tree tree)
{
	IDL_tree ident = IDL_INTERFACE(tree).ident;
	CORBA_TypeCode res;
	char *repo_id = IDL_IDENT_REPO_ID(ident);

	res = find_typecode(repo_id);
	if (res)
		return res;
	res = alloc_typecode();
	res->kind = CORBA_tk_objref;
	res->repo_id = g_strdup(repo_id);
	res->name = g_strdup(IDL_IDENT(ident).str);
	store_typecode(repo_id, res);
	return res;
}

CORBA_TypeCode get_struct_typecode(IDL_tree tree)
{
	IDL_tree ident = IDL_TYPE_STRUCT(tree).ident,
	         members = IDL_TYPE_STRUCT(tree).member_list, tmp1;
	char *repo_id = IDL_IDENT_REPO_ID(ident);
	CORBA_unsigned_long i = 0;

	CORBA_TypeCode res = find_typecode(repo_id);
	if (res)
		return res;
	res = alloc_typecode();
	res->kind = CORBA_tk_struct;
	res->repo_id = g_strdup(repo_id);
	res->name = g_strdup(IDL_IDENT(ident).str);

	// determine number of members
	res->sub_parts = 0;
	tmp1 = members;
	while (tmp1) {
		IDL_tree member = IDL_LIST(tmp1).data;
		IDL_tree dcls = IDL_MEMBER(member).dcls;
		res->sub_parts += IDL_list_length(dcls);
		tmp1 = IDL_LIST(tmp1).next;
	}
	res->subnames = g_new(const gchar *, res->sub_parts);
	res->subtypes = g_new(CORBA_TypeCode, res->sub_parts);
	
	tmp1 = members;
	while (tmp1) {
		IDL_tree member = IDL_LIST(tmp1).data;
		IDL_tree type_spec = IDL_MEMBER(member).type_spec;
		IDL_tree dcls = IDL_MEMBER(member).dcls;
		CORBA_TypeCode base_tc = get_typecode(type_spec);

		IDL_tree tmp2 = dcls;
		while (tmp2) {
			IDL_tree dcl = IDL_LIST(tmp2).data;
			res->subnames[i] = get_declarator_name(dcl);
			res->subtypes[i++] = get_declarator_typecode(dcl, base_tc);
			tmp2 = IDL_LIST(tmp2).next;
		}
		CORBA_Object_release((CORBA_Object)base_tc, NULL);
		tmp1 = IDL_LIST(tmp1).next;
	}
	store_typecode(repo_id, res);
	return res;
}

CORBA_TypeCode get_sequence_typecode(IDL_tree tree)
{
	IDL_tree spec = IDL_TYPE_SEQUENCE(tree).simple_type_spec;
	IDL_tree pic = IDL_TYPE_SEQUENCE(tree).positive_int_const;

	CORBA_TypeCode res = alloc_typecode();
	res->kind = CORBA_tk_sequence;
	res->sub_parts = 1;
	res->subtypes = g_new(CORBA_TypeCode, 1);
	res->subtypes[0] = get_typecode(spec);

	if (pic)
		res->length = IDL_INTEGER(pic).value;
	else
		res->length = 0;
	return res;
}

CORBA_TypeCode get_enum_typecode(IDL_tree tree)
{
	IDL_tree ident = IDL_TYPE_ENUM(tree).ident,
	         elist = IDL_TYPE_ENUM(tree).enumerator_list, tmp;
	char *repo_id = IDL_IDENT_REPO_ID(ident);
	CORBA_unsigned_long i = 0;

	CORBA_TypeCode res = find_typecode(repo_id);
	if (res)
		return res;

	res = alloc_typecode();
	res->name = g_strdup(IDL_IDENT(ident).str);
	res->kind = CORBA_tk_enum;
	res->repo_id = g_strdup(repo_id);

	// Figure out how many members
	res->sub_parts = 0;
	tmp = elist;
	while (tmp) {
		res->sub_parts++;
		tmp = IDL_LIST(tmp).next;
	}
	res->subnames = g_new(const gchar *, res->sub_parts);
	tmp = elist;
	while (tmp) {
		ident = IDL_LIST(tmp).data;
		res->subnames[i++] = g_strdup(IDL_IDENT(ident).str);
		tmp = IDL_LIST(tmp).next;
	}
	store_typecode(repo_id, res);
	return res;
}

int enumerator_index(IDL_tree label)
{
	IDL_tree tmp = IDL_NODE_UP(label);
	int i = 0;
	do {
		tmp = IDL_LIST(tmp).prev;
		i++;
	} while (tmp);
	return i - 1;
}
	
CORBA_TypeCode get_union_typecode(IDL_tree tree)
{
	IDL_tree ident = IDL_TYPE_UNION(tree).ident;
	IDL_tree switch_type = IDL_TYPE_UNION(tree).switch_type_spec;
	IDL_tree switch_body = IDL_TYPE_UNION(tree).switch_body, tmp1;
	CORBA_unsigned_long i = 0;

	char *repo_id = IDL_IDENT_REPO_ID(ident);
	CORBA_TypeCode res = find_typecode(repo_id);
	if (res)
		return res;

	res = alloc_typecode();
	res->kind = CORBA_tk_union;
	res->repo_id = g_strdup(repo_id);
	res->name = g_strdup(IDL_IDENT(ident).str);

	// count arms
	res->sub_parts = 0;
	tmp1 = switch_body;
	while (tmp1) {
		IDL_tree case_stmt = IDL_LIST(tmp1).data;
		IDL_tree labels = IDL_CASE_STMT(case_stmt).labels;
		gint length = 0;

		IDL_tree tmp2 = labels;
		while (tmp2) {
			if (!IDL_LIST(tmp2).data) {
				if (!IDL_LIST(tmp2).prev && !IDL_LIST(tmp2).next)
					length++;
			}
			else length++;
			tmp2 = IDL_LIST(tmp2).next;
		}
		res->sub_parts += length;
		tmp1 = IDL_LIST(tmp1).next;
	}

	res->subnames = g_new(const gchar *, res->sub_parts);
	res->subtypes = g_new(CORBA_TypeCode, res->sub_parts);
	res->sublabels = g_new(CORBA_any, res->sub_parts);
	res->default_index = -1;
	res->discriminator = get_typecode(switch_type);

	tmp1 = switch_body;
	while (tmp1) {
		IDL_tree case_stmt = IDL_LIST(tmp1).data;
		IDL_tree labels = IDL_CASE_STMT(case_stmt).labels,
		         element_spec = IDL_CASE_STMT(case_stmt).element_spec,
		         type_spec = IDL_MEMBER(element_spec).type_spec,
		         dcls = IDL_MEMBER(element_spec).dcls,
		         declarator = IDL_LIST(dcls).data;

		IDL_tree tmp2 = labels;
		while (tmp2) {
			IDL_tree label = IDL_LIST(tmp2).data;
			if (!label) {
				res->default_index = i;
				if (IDL_LIST(tmp2).prev || IDL_LIST(tmp2).next) {
					tmp2 = IDL_LIST(tmp2).next;
					continue;
				}
			}
			res->subnames[i] = get_declarator_name(declarator);
			res->subtypes[i] = get_declarator_typecode(declarator,
			                                           get_typecode(type_spec));
			if (!label) {
				CORBA_octet *val;
				res->sublabels[i]._type = dupe_tc(TC_CORBA_octet);
				res->sublabels[i]._release = TRUE;
				val = g_new(CORBA_octet, 1);
				*val = 0;
				res->sublabels[i]._value = val;
			}
			else {
				res->sublabels[i]._type = dupe_tc(res->discriminator);
				res->sublabels[i]._release = TRUE;
				switch (res->discriminator->kind) {
					C_M(CORBA_tk_enum, CORBA_long, enumerator_index(label));
					C_M(CORBA_tk_long, CORBA_long, IDL_INTEGER(label).value);
					C_M(CORBA_tk_ulong, CORBA_unsigned_long, 
					    IDL_INTEGER(label).value);
					C_M(CORBA_tk_boolean, CORBA_boolean, IDL_INTEGER(label).value);
					C_M(CORBA_tk_char, CORBA_char, *IDL_CHAR(label).value);
					C_M(CORBA_tk_short, CORBA_short, IDL_INTEGER(label).value);
					C_M(CORBA_tk_ushort, CORBA_unsigned_short,
					    IDL_INTEGER(label).value);
					C_M(CORBA_tk_longlong, CORBA_long_long, 
					    IDL_INTEGER(label).value);
					C_M(CORBA_tk_ulonglong, CORBA_unsigned_long_long,
					    IDL_INTEGER(label).value);
					default:
						g_error("Bad union discriminator type %d",
						          res->discriminator->kind);
				}
			}
			tmp2 = IDL_LIST(tmp2).next;
			i++;
		}
		tmp1 = IDL_LIST(tmp1).next;
	}
	store_typecode(repo_id, res);
	return res;
}

CORBA_TypeCode get_float_typecode(IDL_tree tree)
{
	enum IDL_float_type f_type = IDL_TYPE_FLOAT(tree).f_type;
	switch (f_type) {
		case IDL_FLOAT_TYPE_FLOAT:
			return dupe_tc(TC_CORBA_float);
		case IDL_FLOAT_TYPE_DOUBLE:
			return dupe_tc(TC_CORBA_double);
		case IDL_FLOAT_TYPE_LONGDOUBLE:
			return dupe_tc(TC_CORBA_longdouble);
	}
	g_assert_not_reached();
	return NULL;
}

	
CORBA_TypeCode get_typecode(IDL_tree tree)
{
	IDL_tree ident = IDL_INTERFACE(tree).ident;
	char *p = IDL_IDENT(ident).str;
	switch (IDL_NODE_TYPE(tree)) {
		case IDLN_TYPE_ANY:
			return dupe_tc(TC_CORBA_any);
		case IDLN_TYPE_CHAR:
			return dupe_tc(TC_CORBA_char);
		case IDLN_TYPE_BOOLEAN:
			return dupe_tc(TC_CORBA_boolean);
		case IDLN_TYPE_OBJECT:
			return dupe_tc(TC_CORBA_Object);
		case IDLN_TYPE_OCTET:
			return dupe_tc(TC_CORBA_octet);
		case IDLN_TYPE_TYPECODE:
			return dupe_tc(TC_CORBA_TypeCode);
		case IDLN_TYPE_WIDE_CHAR:
			return dupe_tc(TC_CORBA_wchar);

		case IDLN_TYPE_ENUM:
			return get_enum_typecode(tree);
		case IDLN_TYPE_STRUCT:
			return get_struct_typecode(tree);
		case IDLN_TYPE_STRING:
			return get_string_typecode(tree);
		case IDLN_TYPE_INTEGER:
			return get_integer_typecode(tree);
		case IDLN_EXCEPT_DCL:
			return get_exception_typecode(tree);
		case IDLN_IDENT:
			return get_ident_typecode(tree);
		case IDLN_INTERFACE:
			return get_interface_typecode(tree);
		case IDLN_TYPE_SEQUENCE:
			return get_sequence_typecode(tree);
		case IDLN_TYPE_UNION:
			return get_union_typecode(tree);

		case IDLN_TYPE_FLOAT:
			return get_float_typecode(tree);
		case IDLN_TYPE_WIDE_STRING:
			return get_wstring_typecode(tree);
		
		default:
			g_error("Oops!  Typecode %s is Not Yet Implemented!", 
			          IDL_NODE_TYPE_NAME(tree));
	}
}
