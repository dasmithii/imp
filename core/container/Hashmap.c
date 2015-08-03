#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/number.h>
#include <stdint.h>

#define MIN_BUCKET_COUNT 8
#define MAX_LOAD .75
#define MIN_LOAD .1




typedef struct _Buck {
	struct _Buck *next;
	uint64_t khc;  // key hash code
	iObject *v;  // value
} _Buck;


static void _Buck_clean(_Buck *b){
	b = b->next;

	while(b){
		_Buck *tmp = b;
		b = b->next;
		free(tmp);
	}
}


static bool _Buck_put(_Buck *b, uint64_t khc, iObject *v){
	if(b->khc == 0){
		// Make new key in place.
		b->khc = khc;
		b->v = v;
		return true;
	}

beg:
	if(b->khc == khc){
		// Overwrite for key.
		b->v = v; 
		return false;
	} else if(!b->next){
		// Make new key.
		b->next = calloc(1, sizeof(_Buck));
		b->next->khc = khc;
		b->next->v = v;
		return true;
	} else {
		// Recurse without doing an actual function call.
		b = b->next;
		goto beg;
	}
}


static iObject *_Buck_get(_Buck *b, uint64_t khc){
	while(b){
		if(b->khc == khc){
			return b->v;
		}
		b = b->next;
	}
	return NULL;
}


static bool _Buck_remove(_Buck *b, uint64_t khc){
	if(!khc){
		return false;
	}

	if(b->khc == khc){
		b->khc = 0;
		return true;
	}

	while(b->next){
		_Buck *next = b->next;
		if(next->khc == khc){
			b->next = next->next;
			free(next);
			return true;
		}
		b = next;
	}
	return false;
}



typedef struct {
	size_t elemC;
	size_t buckC;
	_Buck *bucks;
} _Map;


static inline float _Map_load(_Map *self){
	return ((float) self->elemC) / ((float) self->buckC);
}
 

static inline void _Map_setBuckC(_Map *m, size_t t){
	_Buck *nBucks = calloc(t, sizeof(_Buck));
	for(size_t i = 0; i < m->buckC; i++){
		_Buck *b = m->bucks + i;
		while(b && b->khc){
			_Buck_put(nBucks + (b->khc % t), b->khc, b->v);
			b = b->next;
		}
		_Buck_clean(m->bucks + i);
	}
	free(m->bucks);
	m->bucks = nBucks;
	m->buckC = t;
}


static inline void _Map_clean(_Map *m){
	for(size_t i = 0; i < m->buckC; i++){
		_Buck_clean(m->bucks + i);
	}
	free(m->bucks);
}


static inline void _Map_compact(_Map *m){
	size_t t = m->elemC / MAX_LOAD;
	if(t < MIN_BUCKET_COUNT){
		t = MIN_BUCKET_COUNT;
	}
	_Map_setBuckC(m, t);
}


static _Map *getRaw(iObject *self){
	return (_Map*) iObject_getDataDeep(self, "__data");
}



iObject *Hashmap_makeEmpty(iRuntime *runtime
	                     , iObject *context
	                     , iObject *module
	                     , int argc
	                     , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Hashmap:makeEmpty does not accept arguments");
	}

	iObject *r = iRuntime_MAKE(runtime, Object);
	iObject_putShallow(r, "#", module);
	_Map *map = malloc(sizeof(_Map));
	if(!map){
		abort();
	}
	map->buckC = MIN_BUCKET_COUNT;
	map->bucks = calloc(MIN_BUCKET_COUNT, sizeof(_Buck));
	if(!map->bucks){
		abort();
	}

	map->elemC = 0;
	iObject_putDataShallow(r, "__data", map);
	return r;
}


iObject *Hashmap_put(iRuntime *runtime
	               , iObject *context
	               , iObject *self
	               , int argc
	               , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context, "Hashmap:put requires exactly 2 arguments");
	}

	_Map *map = getRaw(self);
	const float load = _Map_load(map);
	if(load > MAX_LOAD){
		_Map_setBuckC(map, map->buckC * 2);
	}

	const uint64_t hc = iRuntime_hashCodeOf(runtime, context, argv[0]);
	if(_Buck_put(map->bucks + (hc % map->buckC), hc, argv[1])){
		map->elemC++;
	}
	return NULL;
}



iObject *Hashmap_remove(iRuntime *runtime
	                  , iObject *context
	                  , iObject *self
	                  , int argc
	                  , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Hashmap:remove requires exactly 1 arguments");
	}

	_Map *map = getRaw(self);


	const uint32_t hc = iRuntime_hashCodeOf(runtime, context, argv[0]);
	if(_Buck_remove(map->bucks + (hc % map->buckC), hc)){
		map->elemC--;
		if(map->buckC > MIN_BUCKET_COUNT && _Map_load(map) < MIN_LOAD) {
			_Map_compact(map);
		}
	}

	return NULL;
}


iObject *Hashmap_size(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Hashmap:size does not accept arguments");
	}

	_Map *m = getRaw(self);

	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, (double) m->elemC);
	return r;
}


iObject *Hashmap_containsKey(iRuntime *runtime
	                       , iObject *context
	                       , iObject *self
	                       , int argc
	                       , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Hashmap:containsKey requires exactly 1 argument");
	}

	_Map *map = getRaw(self);
	const uint32_t hc = iRuntime_hashCodeOf(runtime, context, argv[0]);

	iObject *r = iRuntime_MAKE(runtime, Number);

	if(_Buck_get(map->bucks + (hc % map->buckC), hc)){
		iNumber_setRaw(r, 1);
	} else {
		iNumber_setRaw(r, 0);
	}

	return r;
}


iObject *Hashmap_eachValue(iRuntime *runtime
	                     , iObject *context
	                     , iObject *self
	                     , int argc
	                     , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Hashmap:eachValue requires exactly 1 argument");
	}

	_Map *m = getRaw(self);
	
	for(size_t i = 0; i < m->buckC; i++){
		_Buck *b = m->bucks + i;
		while(b && b->khc){
			iRuntime_activate(runtime, context, argv[0], 1, &b->v);
			b = b->next;
		}
	}

	return NULL;
}



iObject *Hashmap_get(iRuntime *runtime
	               , iObject *context
	               , iObject *self
	               , int argc
	               , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Hashmap:get requires exactly 1 arguments");
	}

	_Map *map = getRaw(self);
	const uint32_t hc = iRuntime_hashCodeOf(runtime, context, argv[0]);

	iObject *r = _Buck_get(map->bucks + (hc % map->buckC), hc);
	return r? r:runtime->nil;
}




iObject *Hashmap__markInternalsRecursively(iRuntime *runtime
	                                     , iObject *context
	                                     , iObject *self
	                                     , int argc
	                                     , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Hashmap:_markInternalsRecursively does not accept arguments");
	}

	_Map *m = getRaw(self);
	if(!m){
		return NULL;
	}

	for(size_t i = 0; i < m->buckC; i++){
		_Buck *b = m->bucks + i;
		while(b && b->khc){
			if(b->v->gc_mark == false){
				iRuntime_callMethod(runtime, context, b->v, "_markRecursively", 0, NULL);
			}
			b = b->next;
		}
	}

	return NULL;
}


iObject *Hashmap__clean(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Hashmap:_clean does not accept arguments");
	}

	_Map *m = getRaw(self);
	if(!m){
		return NULL;
	}

	_Map_clean(m);

	return NULL;
}



