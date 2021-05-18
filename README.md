# Assignment Details
 **Submitted by  :** Ajit Pant

 **Enrollment No :** 18312004

 **Assignment 6 : Starve-Free Readers Writers Problem**

 Operating Systems (CSN-232) Spring 2021 (IIT Roorkee)




# Compilation Instructions
    The code depends on C++14 for execution, and has been tested with g++-9 & g++-11. Kindly use the following command for compiling.

    ```
    cd /path/to/folder
    g++-9 -std=c++14  reader_writer.cpp -o reader_writer -pthread
    ./reader_writer
    ```

# Design of the Algorithm
    The basic idea to avoid starvation here, is to run threads in FIFO like order. We know that in FIFO, unless some thread run for indefinite time, all the threads would be getting chance to run and finish their execution.

    To achieve this what is do is to use an ordered lock, which gives access to the callers in the order of their calling time. So the first thread to wait for the lock would be the first one to acquire it.

    This order allows us to start the threads in FIFO order.
    So first thread is run, then if the present one is a reader, we let it run and simultaneouly check if the next one can also run. So if there are multiple readers in a row all would be working simultaneouly. This allows for us to solve this problem in an efficent manner.

# Short Documentation of Some of the functions
## More details about these functions are in the comments in the code file reader_writer.cpp

    1. Ordered Lock: Reference: https://stackoverflow.com/a/14792685
        Since there is no ordered lock in STL for forcing the FIFO order, I have taken the code for it from the above mentioned stackoverflow link. This is a small part of the code only, and the code can work even if we replace this ordered lock with some other lock, just then there would be no guarentee that the code runs in FIFO Order.

    2. ProcessBase:
        It is an abstract base class, from which the ReaderProcess and WriterProcess are derived. There are 5 functions in it     entry(), preCriticalSection(), criticalSection(), postCriticalSection(), remainder(). Thse denote the various parts of the code of the reader and writer.

    3. ReaderProcess and WriterProcess:
        These are inherited from ProcessBase, and contains various part of their functionality seperated into function depending upon whether they can be run with other threads simultaneously or not.
        They also contain their pids (true_pid and writer_pid/reader_pid).

    4. main():
        It spawns 'threadsToSpawn' number of threads, each of which has an (almost) equal chance of being a reader or a writer. It also output the turnaround time of each thread via cout.



Thank you
