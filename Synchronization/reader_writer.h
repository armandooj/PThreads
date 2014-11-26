#ifndef   	_READER_WRITER_H_
#define   	_READER_WRITER_H_

#define BR_EVENT_ID 0
#define ER_EVENT_ID 1
#define BW_EVENT_ID 2
#define EW_EVENT_ID 3

/* The structure is now defined inside reader_writer_X.c so the same
   header can be used for any of your implementation and included
   anywhere in the code */ 
typedef struct reader_writer *reader_writer_t;

reader_writer_t rw_init(); 

void begin_read(reader_writer_t rw, int value);

void end_read(reader_writer_t rw, int value); 

void begin_write(reader_writer_t rw, int value); 

void end_write(reader_writer_t rw, int value); 


#endif 	    /* !_READER_WRITER_H_ */
