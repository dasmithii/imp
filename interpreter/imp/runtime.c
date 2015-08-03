#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <imp/builtin/general.h>
#include <imp/builtin/route.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>
#include <imp/builtin/closure.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/base.h>
#include <imp/builtin/miscellaneous.h>
#include <imp/c.h>
#include <imp/parser.h>
#include <imp/runtime.h>


// Activates <object> with given arguments on the <origin> 
// object. This is particularly useful for method-type
// objects.
iObject *iRuntime_activateOn(iRuntime *runtime
	                     , iObject *context
	                     , iObject *object
	                     , int argc
	                     , iObject **argv
	                     , iObject *origin){
	assert(runtime);
	assert(iObject_isValid(object));

	iObject_reference(context);
	iObject_reference(object);
	for(int i = 0; i < argc; i++){
		iObject_reference(argv[i]);
	}
	iObject_reference(origin);

	iObject *r = NULL;

	iObject *special = iObject_getDeep(object, "_activate");
	iObject *internal = NULL;
	if(special){
		r = iRuntime_activateOn(runtime, context, special, argc, argv, origin);
	} else {
		internal = iObject_getDataDeep(object, "__activate");
		if(!internal){
			iRuntime_throwString(runtime, context, "object not activatable");
			goto fuckit;
		}

		CFunction cf = *((CFunction*) internal);

		if(iObject_hasKeyDeep(object, "__privilege")){

			// execute with arguments that haven't yet been 
			// dereferenced
			r = cf(runtime, context, origin, argc, argv);
			
		} else if(iBuiltin_id(object) == iBUILTIN_CLOSURE){

			// execute with dereferenced arguments prefixed
			// with 'self' for use in closure
			const int argc2 = argc + 1;
			iObject **argv2 = malloc(sizeof(iObject*) * argc2);
			if(!argv2){
				abort();
			}
			argv2[0] = origin;
			for(int i = 1; i < argc2; i++){
				argv2[i] = unroute(argv[i - 1]);
				iObject_reference(argv2[i]);
			}

			r = cf(runtime, context, object, argc2, argv2);  // TODO: deal with this

			for(int i = 1; i < argc2; i++){
				iObject_unreference(argv2[i]);
			}
			free(argv2);

		} else {

			// execute with dereferenced arguments
			iObject **argv2 = malloc(sizeof(iObject*) * argc);
			if(!argv2){
				abort();
			}
			for(int i = 0; i < argc; i++){
				argv2[i] = unroute(argv[i]);
				iObject_reference(argv2[i]);
			}

			r = cf(runtime, context, origin, argc, argv2);  // TODO: deal with this
			
			for(int i = 0; i < argc; i++){
				iObject_unreference(argv2[i]);
			}
			free(argv2);
		}
	}


	fuckit:

	iObject_unreference(context);
	iObject_unreference(object);
	for(int i = 0; i < argc; i++){
		iObject_unreference(argv[i]);
	}
	iObject_unreference(origin);

	if(special || internal){
		return r;
	}
	iRuntime_throwString(runtime, context, "iObject not callable.");
	return NULL;
}


iObject *iRuntime_activate(iRuntime *runtime
	                   , iObject *context
	                   , iObject *object
	                   , int argc
	                   , iObject **argv){
	assert(iObject_isValid(object));
	assert(runtime);
	return iRuntime_activateOn(runtime 
			                , context
			                , object
			                , argc
			                , argv
			                , object);
}


iObject *iRuntime_rawObject(iRuntime *self){
	assert(self);
	return iObjectPool_allocate(self->objectPool);
}


void iRuntime_init(iRuntime *self, char *root, int argc, char **argv){
	assert(self);
	assert(sizeof(double) == 8); // we assume for hashCode methods...

	self->root = root;
	self->argc = argc;
	self->argv = argv;

	self->error = NULL;
	self->objectPool = iObjectPool_forRuntime(self);
	iObjectPool_lockGC(self->objectPool);


	// IMPORTANT: be careful while messing with the order of 
	// builtin initializations (!).

	self->Object = iRuntime_rawObject(self);
	iObject_reference(self->Object);
	iBase_init(self->Object, self);

	self->root_scope = iRuntime_MAKE(self, Object);
	iObject_reference(self->root_scope);
	iObject_putShallow(self->root_scope, "self", self->root_scope);

	#define IMP_INIT_IN_SLOT(OBJECT, NAME)                     \
		self->OBJECT = iRuntime_MAKE(self, Object);             \
		iObject_reference(self->OBJECT);                       \
		iObject_putShallow(self->Object, NAME, self->OBJECT);  \
		i##OBJECT##_init(self->OBJECT, self)
	#define IMP_INIT(NAME) IMP_INIT_IN_SLOT(NAME, #NAME)

	IMP_INIT(String);
	IMP_INIT(Number);
	IMP_INIT(Route);
	IMP_INIT(Closure);
	IMP_INIT_IN_SLOT(Importer, "import");

	self->Array = i_import(self, "core/container/Array", self->root_scope);
	iObject_reference(self->Array);

	iMisc_init(self->Object, self);

	iObjectPool_unlockGC(self->objectPool);
}


iObject *iRuntime_simpleClone(iRuntime *runtime, iObject *base){
	iObject_reference(base);
	iObject *r = iRuntime_rawObject(runtime);
	iObject_unreference(base);

	iObject_putShallow(r, "#", base);
	return r;
}


iObject *iRuntime_clone(iRuntime *runtime, iObject *base){
	assert(runtime);
	assert(iObject_isValid(base));

	if(iObject_hasMethod(base, "~")){
		return iRuntime_callMethod(runtime
			                    , NULL
			                    , base
			                    , "~"
			                    , 0
			                    , NULL);
	} else {
		return iRuntime_simpleClone(runtime, base);
	}
}


iObject *iRuntime_cloneField(iRuntime *runtime, char *field){
	assert(runtime);
	assert(field);
	return iRuntime_clone(runtime, iObject_getDeep(runtime->root_scope, field));
}


static iObject *iRuntime_tokenToObject(iRuntime *self, iObject *scope, iToken *token){
	assert(self);
	assert(iObject_isValid(scope));
	assert(token);

	iObject *r = NULL;
	iObject_reference(scope);

	switch(token->type){
	case iTOKEN_ROUTE:
		{
			iObject *route = iRuntime_clone(self, self->Route);
			assert(token->data.text);
			iRoute_setRaw(route, token->data.text);
			iRoute_setContext(route, scope);
			r = route;
			break;
		}
	case iTOKEN_NUMBER:
		{
			iObject *number = iRuntime_cloneField(self, "Number");
			iNumber_setRaw(number, token->data.number);
			r = number;
			break;
		}
	case iTOKEN_STRING:
		{
			iObject *str = iRuntime_cloneField(self, "String");
			iString_setRaw(str, token->data.text);
			r = str;
			break;
		}
	default:
		break;
	}

	iObject_unreference(scope);
	return r;
}


void iRuntime_setReturnValue(iRuntime *self, iObject *value){
	assert(self);
	// value == null is allowed
	self->lastReturnValue = value;
	self->returnWasCalled = true;
}


void iRuntime_clearReturnValue(iRuntime *self){
	assert(self);
	self->lastReturnValue = NULL;
	self->returnWasCalled = false;
}


iObject *iRuntime_executeInContext(iRuntime *runtime
	                             , iObject *scope
	                             , iParseNode node){
	assert(runtime);
	assert(iObject_isValid(scope));
	iObject *r = NULL;

	iObject_reference(scope);


	switch(node.type){
	case iNODE_LEAF:
		{
			r = iRuntime_tokenToObject(runtime, scope, node.token);
			if(iToken_isContextualRoute(node.token)){
				iObject_reference(r);
				iRoute_setContext(r, iRuntime_executeInContext(runtime, scope, node.argv[0]));
				iObject_unreference(r);
			}
			break;
		}
	case iNODE_BLOCK:
		{
			iRuntime_clearReturnValue(runtime);


			for(size_t i = 0; i < node.argc; i++){
				iRuntime_executeInContext(runtime
					                   , scope
					                   , node.argv[i]);
				if(runtime->returnWasCalled){
					break;
				}
			}
			r = runtime->lastReturnValue;
			iRuntime_clearReturnValue(runtime);
		}
		break;
	case iNODE_CALL:
		{
			// iterate through parse node... TODO: mark these in collection
			iObject **subs = malloc(node.argc * sizeof(iObject*));
			if(!subs){
				abort();
			}
			for(size_t i = 0; i < node.argc; i++){
				subs[i] = iRuntime_executeInContext(runtime
					                             , scope
					                             , node.argv[i]);

				if(subs[i]){
					iObject_reference(subs[i]);
				}
			}

			const int argc = node.argc - 1;
			iObject **argv =  subs + 1;

			r = iRuntime_activate(runtime
	               , scope
	               , subs[0]
	               , argc
	               , argv);

			for(size_t i = 0; i < node.argc; i++){
				if(subs[i]){
					iObject_unreference(subs[i]);
				}
			}

			free(subs);
		}
		break;
	case iNODE_OBJECT:
		{
			r = iRuntime_MAKE(runtime, Object);
			iObject_reference(r);
			if(node.argc % 2 != 0){
				printf("%zu\n", node.argc);
				iRuntime_throwString(runtime, scope, "object literal requires pairs of inputs");
			}
			for(size_t i = 0; i < node.argc; i += 2){
				iObject *args[2];
				args[0] = iRuntime_executeInContext(runtime, scope, node.argv[i]);
				if(iBuiltin_id(args[0]) != iBUILTIN_ROUTE){
					iRuntime_throwString(runtime, scope, "object literals require atom-value pairs");
				}
				iObject_reference(args[0]);
				args[1] = iRuntime_executeInContext(runtime, scope, node.argv[i+1]);
				iObject_unreference(args[0]);
				iRuntime_callMethod(runtime
					             , r   
					             , r
					             , "def"
					             , 2
					             , args);
			}
			iObject_unreference(r);
		}
		break;
	case iNODE_CLOSURE:
		{
			r = iRuntime_MAKE(runtime, Closure);
			iObject_reference(r);
			iClosure_compile(runtime, r, &node, scope);
			iObject_unreference(r);
		}
		break;
	}

	iObject_unreference(scope);

	return r;
}


iObject *iRuntime_executeSourceInContext(iRuntime *self
	                                   , char *code
	                                   , iObject *context){
	assert(self);
	assert(code); // todo ebnf check code

	iParseTree tree;
	iObject *r = NULL;

	int rc = iParseTree_init(&tree, code);
	if(rc){
		iRuntime_throwString(self, context, tree.error);
	} else {
		if(!iObject_hasKeyDeep(context, "self")){
			iObject_putShallow(context, "self", context);
		}
		r = iRuntime_executeInContext(self, context, tree.root);
	}

	iParseTree_clean(&tree);
	return r;
}


iObject *iRuntime_executeFile(iRuntime *self, char *path){
	return iRuntime_executeFileInContext(self, path, self->root_scope);
}


static char *readFile(char *path){
	assert(path);
	FILE *stream;
	char *contents;
	size_t fileSize = 0;
	stream = fopen(path, "rb");
	if(!stream){
		return NULL;
	}
	fseek(stream, 0L, SEEK_END);
	fileSize = ftell(stream);
	fseek(stream, 0L, SEEK_SET);
	contents = malloc(fileSize+1);
	size_t size=fread(contents,1,fileSize,stream);
	contents[size]=0; 
	fclose(stream);
	return contents;
}


iObject *iRuntime_executeFileInContext(iRuntime *self, char *path, iObject *context){
	char *code = readFile(path);
	if(!code){
		iRuntime_throwFormatted(self, context, "failed to read file: '%s'", path);
	}
	char fileSetCom[256];
	// TODO: check path length.
	sprintf(fileSetCom, "(def _FILE '%s')", path);
	iRuntime_executeSourceInContext(self, fileSetCom, context);
	return iRuntime_executeSourceInContext(self, code, context);
}


iObject *iRuntime_executeSource(iRuntime *self, char *code){
	return iRuntime_executeSourceInContext(self, code, self->root_scope);
}


void iRuntime_throw(iRuntime *runtime, iObject *context, iObject *exception){
	iObject_reference(context);
	iObject_reference(exception);
	iObject *lib = i_import(runtime, "core/exceptions", context);
	iObject_unreference(context);
	iObject_unreference(exception);

	iRuntime_callMethod(runtime
		             , context
		             , lib
		             , "throw"
		             , 1
		             , &exception);
}


void iRuntime_throwString(iRuntime *runtime, iObject *context, char *exception){
	assert(runtime);
	assert(exception);

	iObject_reference(context);
	iObject *obj = iRuntime_MAKE(runtime, String);
	iString_setRaw(obj, exception);
	iObject_unreference(context);

	iRuntime_throw(runtime, context, obj);
}


void iRuntime_throwFormatted(iRuntime *runtime, iObject *context, const char *format, ...){
    va_list args;
    va_start(args, format);

    char str[256];
    vsprintf(str, format, args);
    iRuntime_throwString(runtime, context, str);

    fprintf(stderr, ".\n");
    va_end(args);
}


void iRuntime_print(iRuntime *runtime, iObject *context, iObject *object){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iObject_isValid(object));

	iObject_reference(context);
	iObject_reference(object);

	iObject *special = iObject_getDeep(object, "_print");
	void *internal = iObject_getDataDeep(object, "__print");
	if(special){
		iRuntime_activateOn(runtime 
			             , context
			             , special
			             , 0
			             , NULL
			             , object);
	} else if(internal){
		CFunction cf = *((CFunction*) internal);
		cf(runtime, context, object, 0, NULL);
	} else {
		iObject_print(object);
	}

	iObject_unreference(context);
	iObject_unreference(object);
}


iObject *iRuntime_callSpecialMethod(iRuntime *runtime
	                              , iObject *context
	                              , iObject *object
	                              , char *methodName
	                              , int argc
	                              , iObject **argv){
	// try <object>:_<methodName>
	char buf[64];
	sprintf(buf, "_%s", methodName);
	iObject *method = iObject_getDeep(object, buf);
	if(method){
		return iRuntime_activateOn(runtime, context, method, argc, argv, object);
	}
	iRuntime_throwFormatted(runtime, context, "method '%s' does not exist", buf);
	return NULL;
}


iObject *iRuntime_callMethod(iRuntime *runtime
	                       , iObject *context
	                       , iObject *object
	                       , char *methodName
	                       , int argc
	                       , iObject **argv){
	// try <object>:<methodName>
	iObject *method = iObject_getDeep(object, methodName);
	if(method){
		return iRuntime_activateOn(runtime, context, method, argc, argv, object);
	}
	iRuntime_throwFormatted(runtime, context, "method '%s' does not exist", methodName);
	return NULL;
}


uint64_t iRuntime_hashCodeOf(iRuntime *runtime, iObject *context, iObject *object){
	iObject *hco = iRuntime_callMethod(runtime
		                             , context
		                             , object
		                             , "_hashCode"
		                             , 0, NULL);
	if(iBuiltin_id(hco) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "_hashCode method did not return number");
	}
	return *((uint64_t*) iObject_getDataDeep(hco, "__data"));
}

