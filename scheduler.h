// TODO
// #ifndef IMP_SCHEDULER_H_
// #define IMP_SCHEDULER_H_
// #include <unistd.h>

// typedef enum {
// 	COROUTINE_SLEEP,
// 	COROUTINE_YIELD,
// 	COROUTINE_FINISH,
// 	COROUTINE_THROW
// } CoroutineStatusType;


// typedef struct {
// 	CoroutineStatusType type;
// 	union{
// 		int microsecond_delay;
// 		Object *exception;
// 	} data;
// } CoroutineStatus;


// typedef struct {
// 	CoroutineStatus (*step)(void *data);
// 	void (*cleanup)(void *data);
// 	void *data;
// 	Object *error;
// } Coroutine;


// // The scheduler will eventually multiplexes coroutines accross
// // a pool of processes, but for now, all it does is schedule 
// // coroutine operation within the context of one thread.
// typedef struct {

// } Scheduler;

// void Scheduler_init(Scheduler *self);
// void Scheduler_step(Scheduler *self);
// int  Scheduler_register(Scheduler *self, Coroutine coro);
// void Scheduler_clean();




// #endif