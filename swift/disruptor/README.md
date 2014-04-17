Disruptor
=========

C++ simple implementations of the LMAX disruptor.


API
=========

   * *ring_buffer<T,Size>*  is a circular buffer with Power of 2 Size
   * *write_cursor*         tracks a position in the buffer and can follow
                            other read cursors to ensure things don't wrap.
   * *shared_write_cursor*  is a write_cursor that may be used from multiple threads
                            other read cursors to ensure things don't wrap.
   * *read_cursor*          tracks the read position and can follow / block
                            on other cursors (read or write).


The concept of the cursors are separated from the data storage.  Every cursor
should read from one or more sources and write to its own outbut buffer.  

Features
==========
  * Progressive-backoff blocking.  When one cursor needs to wait on another it starts
out with a busy wait, followed by a yield, and ultimately falls back to sleeping
wait if the queue is stalled.  

  * Batch writing / Reading with 'iterator like' interface.  Producers and consumers
  always work with a 'range' of valid positions.   The ring buffer provides the
  ability to detect 'wraping' and therefore it should be possible to use this as
  a data queue for a socket that can read/write many slots all at once.  Slots
  could be single bytes and the result would be a very effecient stream-processing
  library.  


