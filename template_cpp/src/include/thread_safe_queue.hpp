/*
Slightly modified code from https://www.educba.com/c-thread-safe-queue/
*/
#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <atomic>
#include <thread>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <stdexcept>
#include <assert.h>

template <typename T>
class ThreadSafeQueue{

private:

    std::atomic<size_t> curr_size;
    size_t max_size;    // max number of items in the queue
    std::condition_variable cv_push, cv_pop; // condition variables for push() pop() operations
    std::mutex mutex;       // mutex for locking the queue
    std::queue<T> queue;


public:

    size_t getSize(){
        return curr_size;
    }

    // create and initialize the Q with maximum size
    explicit ThreadSafeQueue(size_t i_max_size = 512) : curr_size(0), max_size(i_max_size){}

    //default destructor
    //default copy constructor

    /* insert element at the end of the queue,
       if the queue is full, wait for some other thread (consumer) 
       to remove an element from the queue
     */
    void push(T const elem){
        std::unique_lock<std::mutex> lock(mutex); // creates lock and calls mutex.lock()
        while (curr_size == max_size){
            //Atomically unlocks lock, blocks the current executing thread, 
            //and adds it to the list of threads waiting on *this
            //The thread will be unblocked when notify_all() or notify_one() is executed.
            //It may also be unblocked spuriously
            cv_push.wait(lock);
        }
        curr_size += 1;
        queue.push(elem);
        // If any threads are waiting on *this, calling notify_one 
        // unblocks one of the waiting threads. A consumer thread in this case
        cv_pop.notify_all();
    }


    /* remove and return element from the head of the queue, if the 
        queue is empty, current thread waits for some other thread (producer)
        to insert an element
     */
    T pop(){
        std::unique_lock<std::mutex> lock(mutex);
        while (curr_size == 0){
            // wait for the queue to be non empty
            cv_pop.wait(lock);
        }
        curr_size -= 1;
        assert(curr_size >= 0 && curr_size <= max_size);
        T elem = queue.front();
        queue.pop();
        cv_push.notify_all();
        return elem;
    }


};

#endif