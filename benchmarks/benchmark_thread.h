#ifndef BENCHMARK_THREAD_H_
#define BENCHMARK_THREAD_H_

#define BENCHMARK_THREAD

#ifdef BENCHMARK_THREAD
#include "mbed.h"

static void print_thread_data(Thread *t, Serial *pc) {
    pc->printf("Thread %s state: %d,\tPriority: %d\r\n", t->get_name(), t->get_state(), t->get_priority());
    pc->printf("\tStack size:\t%d bytes\r\n\tMax. use:\t%d bytes\r\n", t->stack_size(), t->max_stack());
}
static void print_thread_stack(Thread *t, Serial *pc) {
    pc->printf("Thread %s max. use:\t%d bytes\r\n", t->get_name(), t->max_stack());
}
#else
static void print_thread_data(Thread *t, Serial *pc) {}
static void print_thread_stack(Thread *t, Serial *pc) {}
#endif // BENCHMARK_THREAD

#endif // BENCHMARK_THREAD_H_
