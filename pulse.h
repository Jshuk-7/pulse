#ifndef PULSE_H
#define PULSE_H

#include <pthread.h>
#include <stdint.h>
#include <string.h>

/*
 * MIT License
 * 
 * Copyright (c) 2024 Jshuk-7
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

/*
 * To begin using the pulse library:
 *
 * 1. Copy and paste this file into your project
 *
 * 2. In 'one' .c or .cpp file type the following lines
 *    #define PULSE_IMPLEMENTATION
 *    #include "pulse.h"
 *
 * 3. Then whenever you need access to the library in another .c/.cpp file
 *    just '#include "pulse.h"' and you're off!
 *
 * Things to note:
 * 
 * 1. You can '#define PULSE_MAX_THREADS' to a whatever you want. It defaults
 *    to 100 threads per pool.
 * 
 */

#define PULSE_MAX_THREADS 100

typedef void* (*thread_fn_t)(void*);

typedef struct pulse_thread_t
{
    pthread_t handle;
    int32_t active;
    const char* name;
} pulse_thread_t;

typedef struct pulse_pool_t
{
    pulse_thread_t threads[PULSE_MAX_THREADS];
} pulse_pool_t;

typedef enum pulse_error_t
{
    PULSE_ERROR_NONE,
    PULSE_ERROR_MAX_THREADS_REACHED,
} pulse_error_t;

void            pulse_pool_init(pulse_pool_t* pool);

int32_t         pulse_pool_next_thread(pulse_pool_t* pool, const char* name, thread_fn_t fn, void* arg);

pulse_thread_t* pulse_pool_get_thread_by_name(pulse_pool_t* pool, const char* name);

void*           pulse_pool_join_thread(pulse_pool_t* pool, pulse_thread_t* thread);

void            pulse_pool_detach_thread(pulse_pool_t* pool, pulse_thread_t* thread);

void            pulse_pool_cancel_thread(pulse_pool_t* pool, pulse_thread_t* thread);

size_t          pulse_pool_get_active_count(pulse_pool_t* pool);

size_t          pulse_pool_get_free_count(pulse_pool_t* pool);

const char*     pulse_strerror(int32_t err);

#endif

#ifdef PULSE_IMPLMENTATION

void pulse_pool_init(pulse_pool_t* pool)
{
    if (pool == NULL)
    {
        return;
    }

    for (size_t i = 0; i < PULSE_MAX_THREADS; i++)
    {
        pulse_thread_t* thread = &pool->threads[i];
        thread->active = 0;
		thread->name = "";
    }
}

int32_t pulse_pool_next_thread(pulse_pool_t* pool, const char* name, thread_fn_t fn, void* arg)
{
    if (pool == NULL)
    {
        return PULSE_ERROR_NONE;
    }

    for (size_t i = 0; i < PULSE_MAX_THREADS; i++)
    {
        pulse_thread_t* thread = &pool->threads[i];

        if (thread->active)
        {
            continue;
        }

        thread->active = 1;
        thread->name = name;
        pthread_create(&thread->handle, NULL, fn, arg);
        return PULSE_ERROR_NONE;
    }

    return PULSE_ERROR_MAX_THREADS_REACHED;
}

pulse_thread_t* pulse_pool_get_thread_by_name(pulse_pool_t* pool, const char* name)
{
	if (pool == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < PULSE_MAX_THREADS; i++)
	{
		pulse_thread_t* thread = &pool->threads[i];

		if (strlen(thread->name) != strlen(name))
		{
			continue;
		}

		if (strcmp(thread->name, name) == 0)
		{
			return thread;
		}
	}

    return NULL;
}

void* pulse_pool_join_thread(pulse_pool_t* pool, pulse_thread_t* thread)
{
	if (pool == NULL || thread == NULL || !thread->active)
	{
		return NULL;
	}

	pulse_thread_t* temp = pulse_pool_get_thread_by_name(pool, thread->name);
	if (temp != thread)
	{
		return NULL;
	}

	thread->active = 0;
	thread->name = "";
	
	void* retval = NULL;

	pthread_join(thread->handle, &retval);
	return retval;
}

void pulse_pool_detach_thread(pulse_pool_t* pool, pulse_thread_t* thread)
{
	if (pool == NULL || thread == NULL || !thread->active)
	{
		return;
	}

	pulse_thread_t* temp = pulse_pool_get_thread_by_name(pool, thread->name);
	if (temp != thread)
	{
		return;
	}

	thread->active = 0;
	thread->name = "";
	pthread_detach(thread->handle);
}

void pulse_pool_cancel_thread(pulse_pool_t* pool, pulse_thread_t* thread)
{
	if (pool == NULL || thread == NULL || !thread->active)
	{
		return;
	}

	pulse_thread_t* temp = pulse_pool_get_thread_by_name(pool, thread->name);
	if (temp != thread)
	{
		return;
	}

	thread->active = 0;
	thread->name = "";
	pthread_cancel(thread->handle);
}

size_t pulse_pool_get_active_count(pulse_pool_t *pool)
{
    if (pool == NULL)
    {
        return 0;
    }

	size_t active_threads = 0;

    for (size_t i = 0; i < PULSE_MAX_THREADS; i++)
    {
		pulse_thread_t* thread = &pool->threads[i];

		if (!thread->active)
		{
			continue;
		}

		active_threads++;
    }

	return active_threads;
}

size_t pulse_pool_get_free_count(pulse_pool_t* pool)
{
    if (pool == NULL)
	{
		return 0;
	}

	size_t active_threads = pulse_pool_get_active_count(pool);
	return PULSE_MAX_THREADS - active_threads;
}

const char* pulse_strerror(int32_t err)
{
    switch ((pulse_error_t)err)
    {
        case PULSE_ERROR_NONE:                return "No error";
        case PULSE_ERROR_MAX_THREADS_REACHED: return "Pool reached maximum number of threads";
    }

    return "unknown error!";
}

#endif