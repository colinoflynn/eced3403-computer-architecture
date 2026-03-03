#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f3xx.h"


/* Queue example from https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/ */

// Opaque circular buffer structure
typedef struct circular_buf_t circular_buf_t;

// Handle type, the way users interact with the API
typedef circular_buf_t* cbuf_handle_t;

/// Pass in a storage buffer and size
/// Returns a circular buffer handle
cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size);

/// Free a circular buffer structure.
/// Does not free data buffer; owner is responsible for that
void circular_buf_free(cbuf_handle_t me);

/// Reset the circular buffer to empty, head == tail
void circular_buf_reset(cbuf_handle_t me);

/// Put version 1 continues to add data if the buffer is full
/// Old data is overwritten
void circular_buf_put(cbuf_handle_t me, uint8_t data);

/// Put Version 2 rejects new data if the buffer is full
/// Returns 0 on success, -1 if buffer is full
int circular_buf_put2(cbuf_handle_t me, uint8_t data);

/// Retrieve a value from the buffer
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_get(cbuf_handle_t me, uint8_t * data);

/// Returns true if the buffer is empty
bool circular_buf_empty(cbuf_handle_t me);

/// Returns true if the buffer is full
bool circular_buf_full(cbuf_handle_t me);

/// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t me);

/// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t me);


// The hidden definition of our circular buffer structure
struct circular_buf_t {
	uint8_t * buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	bool full;
};


#define assert(a)  {if (a == 0){printf("ASSERT ERROR: %d @ line %d in %s\n", (int)a, __LINE__, __FILE_NAME__); __BKPT(1); while(1);}}

cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size)
{
	assert(buffer && size);

	cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
	assert(cbuf);

	cbuf->buffer = buffer;
	cbuf->max = size;
	circular_buf_reset(cbuf);

	assert(circular_buf_empty(cbuf));

	return cbuf;
}

void circular_buf_reset(cbuf_handle_t me)
{
    assert(me);

    me->head = 0;
    me->tail = 0;
    me->full = false;
}

void circular_buf_free(cbuf_handle_t me)
{
	assert(me);
	free(me);
}

bool circular_buf_full(cbuf_handle_t me)
{
	assert(me);

	return me->full;
}

bool circular_buf_empty(cbuf_handle_t me)
{
	assert(me);

	return (!me->full && (me->head == me->tail));
}

size_t circular_buf_capacity(cbuf_handle_t me)
{
	assert(me);

	return me->max;
}

size_t circular_buf_size(cbuf_handle_t me)
{
	assert(me);

	size_t size = me->max;

	if(!me->full)
	{
		if(me->head >= me->tail)
		{
			size = (me->head - me->tail);
		}
		else
		{
			size = (me->max + me->head - me->tail);
		}
	}

	return size;
}

static void advance_pointer(cbuf_handle_t me)
{
	assert(me);

	if(me->full)
   	{
		me->tail = (me->tail + 1) % me->max;
	}

	me->head = (me->head + 1) % me->max;
	me->full = (me->head == me->tail);
}

static void retreat_pointer(cbuf_handle_t me)
{
	assert(me);

	me->full = false;
	if (++(me->tail) == me->max)
	{
		me->tail = 0;
	}
}

void circular_buf_put(cbuf_handle_t me, uint8_t data)
{
	assert(me && me->buffer);

    me->buffer[me->head] = data;

    advance_pointer(me);
}

int circular_buf_put2(cbuf_handle_t me, uint8_t data)
{
    int r = -1;

    assert(me && me->buffer);

    if(!circular_buf_full(me))
    {
        me->buffer[me->head] = data;
        advance_pointer(me);
        r = 0;
    }

    return r;
}

int circular_buf_get(cbuf_handle_t me, uint8_t * data)
{
    assert(me && data && me->buffer);

    int r = -1;

    if(!circular_buf_empty(me))
    {
        *data = me->buffer[me->tail];
        retreat_pointer(me);

        r = 0;
    }

    return r;
}

#define BUFFER_SIZE 128

uint8_t buffer[BUFFER_SIZE];
cbuf_handle_t circ_buffer;

static volatile uint32_t produced = 0;
static volatile uint32_t consumed = 0;
static volatile uint32_t drops    = 0;
static volatile uint32_t errors   = 0;

static inline uint8_t next_byte(void) {
    return (uint8_t)(produced & 0xFFu);
}


void SysTick_Handler(void) {
    if(circular_buf_put2(circ_buffer, next_byte()) == 0){
      produced++;
    }
}

int main(void) {

    circ_buffer = circular_buf_init(buffer, BUFFER_SIZE);

    // SysTick setup: adjust rate to make failures appear quickly
    // Default of 20000 shouldn't have errors, change to 150 to see errors
    SysTick->LOAD = 20000; // Defines interrupt speed
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    uint8_t b;
    uint8_t expected = 0;

    while (1) {
        // “Work” to create timing variability
        for (volatile int i = 0; i < (consumed & 0x0F); i++) { __asm volatile("nop"); }

        if (circular_buf_get(circ_buffer, &b) == 0) {
            consumed++;

            // Check stream consistency: should be sequential mod 256
            if (b != expected) {
                errors++;
                expected = b;      // resync so we can keep counting errors
            }

            expected++;
        }

    }
}