#ifdef __cplusplus
extern "C" {
#endif
#include <Python.h>
#include <orb/orbit.h>

#include "CORBA_ORB.h"
#include "CORBA_TypeCode.h"
#include "CORBA_any.h"
#include "CORBA_fixed.h"
#include "PortableServer.h"
#include "types.h"

#define __DEBUG_LEVEL__ 0

// TODO: this file could probably use some restructuring.

typedef struct {
	PyObject_HEAD
	CORBA_Object obj;
	CORBA_Environment ev;
	GSList *last_method;
	char *repo_id;
} CORBA_PyObject;

void CORBA_PyObject__dealloc(CORBA_PyObject *);
int CORBA_PyObject__setattr(CORBA_PyObject *, char *, PyObject *);
PyObject *CORBA_PyObject__getattr(CORBA_PyObject *, char *);
CORBA_PyObject *CORBA_Object_to_PyObject(CORBA_Object);

typedef struct {
	PyTypeObject *type, *servant_type;
	PyMethodDef *generic_method;
	CORBA_InterfaceDef_FullInterfaceDescription *desc;
} CORBA_PyObject_Glue;

typedef struct {
	void *_private;
	PortableServer_ServantBase__vepv *vepv;

	CORBA_PyObject_Glue *glue;
	PyObject *impl;
} PyOrbit_Servant;

typedef struct {
	PyObject_HEAD
	PyOrbit_Servant *servant;

	CORBA_unsigned_long class_id;
	PortableServer_ClassInfo *class_info;
	CORBA_boolean activated;
	POA_PyObject *poa;
	PortableServer_ObjectId *oid;
} Servant_PyObject;

#define CORBA_PyObject_Check(v) ((v)->ob_type == &CORBA_PyObject_Type)

void Servant_PyObject__dealloc(Servant_PyObject *);
int Servant_PyObject__setattr(Servant_PyObject *, char *, PyObject *);
PyObject *Servant_PyObject__getattr(Servant_PyObject *, char *);


// Globals

extern PyObject *ErrorObject, *ModuleDict, *MainDict,
                *SystemExcept, *UserExcept, *poa_instances,
                *except_base_class, *main_module;
extern GHashTable *object_glue, *type_codes, *exceptions;

	GIOPConnection *demarshal_call(CORBA_Object, GIOPConnection *,
	                          GIOP_unsigned_long,
	                          CORBA_OperationDescription *,
	                          PyObject *, GPtrArray *, PyObject **);
	GPtrArray *marshal_call(CORBA_Object, GIOPConnection *, GIOP_unsigned_long,
	                        CORBA_OperationDescription *, PyObject *);
	CORBA_boolean parse(char *, char *);

	CORBA_boolean marshal_arg(PyObject *, GIOPSendBuffer *, CORBA_TypeCode);
	CORBA_exception_type marshal_exception(PyObject *, PyObject *, 
	                                       GIOPSendBuffer *, CORBA_TypeCode, 
	                                       CORBA_OperationDescription *);
	PyObject *demarshal_arg(GIOPRecvBuffer *, CORBA_TypeCode);
	void demarshal_exception(GIOPRecvBuffer *, CORBA_TypeCode,
	                         CORBA_exception_type, CORBA_OperationDescription *);




void init_server();
// typecode functions
void init_typecodes();
CORBA_TypeCode find_typecode(char *);
char *find_repo_id_from_typecode(CORBA_TypeCode);
void store_typecode(char *, CORBA_TypeCode);

// exception functions
CORBA_boolean check_corba_ex(CORBA_Environment *);
void init_exceptions();
void raise_user_exception(char *, PyObject *);
void raise_system_exception(char *, CORBA_unsigned_long, 
                            CORBA_completion_status, char *, ...);
PyObject *UserExcept_PyClass__init(PyObject *, PyObject *, PyObject *);



PyObject *stub_func(CORBA_PyObject *, PyObject *);
PyObject *get_attribute(CORBA_PyObject *, CORBA_AttributeDescription *);
int set_attribute(CORBA_PyObject *, CORBA_AttributeDescription *, PyObject *);

PyObject *create_poa_stub(PyObject *, PyObject *);
PyObject *new_poa_servant(PyObject *, PyObject *);

CORBA_AttributeDescription *find_attribute(CORBA_PyObject_Glue *glue, char *n);
CORBA_OperationDescription *find_operation(CORBA_PyObject_Glue *glue, char *n);



#define dupe_tc(a) (CORBA_TypeCode)CORBA_Object_duplicate((CORBA_Object)a, NULL)
#define d_message(l, f, a...) \
   if (__DEBUG_LEVEL__ >= l) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, f, ##a)

#ifdef __cplusplus
}
#endif
