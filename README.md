# asn2

usage: ./s-talk myPort RemoteMachineName remotePort

Cancels on "!" and returns 0 on successful termination of each thread.

*****Issues********:
Shutting down threads creates a "still reachable" memory leak. This is partially because some threads are cancelled before being able to free their malloc, but after attempting to clean up the malloc (using the solution stated below), some leaks from "pthread_cancel" and "pthread_cancel_init" still remain.

Tried using pthread_cleanup_push() and a free_malloc handler to clean this up whenever a thread is cancelled but this created some unpredictable behaviour, as well as some invalid frees when run on local machine (own port/machine send to self). Also this solution only cleaned up part of the leak that was caused by malloc, not the part that was supposedly caused by pthread_cancel.
