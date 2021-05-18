/* Submitted by  : Ajit Pant
 * Enrollment No : 18312004
 * Assignment 6 Starve-Free Readers Writers Problem
 * Operating Systems (CSN-232) Spring 2021 (IIT Roorkee)
 * 
 */


#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>
#include <chrono>
#include <random>

#include <mutex>
#include <condition_variable>
#include <queue>
 

// Definitons of reader and writer classes, classes are used for convenience


/* To keep things simple for demo, 'data' is an int variable which is used
 * by all the readers and writers. Each process instance stores a reference to this.
 */




// I couldn't find an ordered lock in STL, so using a manually written version

// Reference: https://stackoverflow.com/a/14792685
class ordered_lock {
    std::queue<std::condition_variable *> cvar;
    std::mutex                            cvar_lock;
    bool                                  locked;
public:
    ordered_lock() : locked(false) {};
    void acquire() {
        std::unique_lock<std::mutex> acquire(cvar_lock);
        if (locked) {
            std::condition_variable signal;
            cvar.emplace(&signal);
            signal.wait(acquire);
        } else {
            locked = true;
        }
    }
    void release() {
        std::unique_lock<std::mutex> acquire(cvar_lock);
        if (cvar.empty()) {
            locked = false;
        } else {
            cvar.front()->notify_one();
            cvar.pop();
        }
    }
};


// Declaring these inside the class is not working, I am not sure why
ordered_lock lockResource, lockReaderCnt, lockFIFO;
/* FIFO lock is for making sure that the processes are used in FIFO order
 * This is used to prevent starvation, as all process would now be getting time,
 * unless some process blocks indefinitely.
 */

ordered_lock lockPID, lockCout;
/* These two lock lockPID and lockCout are not necessary but used here to keep the output in
 * a better format;
 */

int reader_cnt = 0;//Count of currently running readers

class ProcessBase{




public: // Keeping everything public for keeping code simple
    static int global_pid ;
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point finishTime;
    double turnaroundTime; // in seconds

public:
    virtual void entry() = 0 ;
    virtual void preCriticalSection() = 0 ;
    virtual void criticalSection() = 0 ;
    virtual void postCriticalSection() = 0 ;
    virtual void remainder() = 0;
};
int ProcessBase::global_pid = 0;


class ReaderProcess : public ProcessBase{

public: // Everything public for simplicity
    int& data;

    static int total_reader_count;// Stores the total no of readers that exist
    int reader_pid; // Stores the process id of the reader, some readers pid might 
                    // clash with pid of some other writer, it is designed for 
                    // convience of reading the output of the program.
    int true_pid; // Unique pid, does not clash between readers and writers
public:
    ReaderProcess(int & _data): data(_data){
        lockPID.acquire();
            reader_pid = total_reader_count++;
            true_pid =  global_pid++;
        lockPID.release();
    }


    void entry(){
        // No thing to do 

        lockCout.acquire();
            std::cout<<"[Reader: " + std::to_string(reader_pid) +", PID: " + std::to_string(true_pid)+"] Started Reading."<<std::endl;
        lockCout.release();
        startTime =  std::chrono::high_resolution_clock::now();

    }
    void preCriticalSection(){
        // Handle the lock


        lockFIFO.acquire(); // Ensures a FIFO order on execution of processes

        lockReaderCnt.acquire();

            if(reader_cnt == 0)
                lockResource.acquire(); // Wait till the resouce lock is used by writer
            reader_cnt++;

        lockReaderCnt.release();

        lockFIFO.release(); // Readers can release it earlier as multiple reader
                            // can run concurrently



    }

    void criticalSection(){

        using namespace std::literals;
        std::this_thread::sleep_for(100ms);

        lockCout.acquire();
            std::cout<<"[Reader: " + std::to_string(reader_pid) +", PID: " + std::to_string(true_pid)+"] Value read as: " + std::to_string(data)<<std::endl;
        lockCout.release();

    }
    void postCriticalSection(){
        // Handle the lock

        lockReaderCnt.acquire();

            reader_cnt--;
            if(reader_cnt == 0)
                lockResource.release(); // Release the resource lock for other writer/reader

        lockReaderCnt.release();

    }
    void remainder(){

        lockCout.acquire();
            std::cout<<"[Reader: " + std::to_string(reader_pid) +", PID: " + std::to_string(true_pid)+"] Finished reading."<<std::endl;
        lockCout.release();
        finishTime =  std::chrono::high_resolution_clock::now();
    }


};
int ReaderProcess::total_reader_count = 0; // Stores the total no of readers that exist

class WriterProcess : public ProcessBase{

public: //Keeping everything public for simplicity 
    int& data;

    static int total_writer_count;// Stores the total no of writers that exist
    int writer_pid; // Stores the process id of the writer, some readers pid might 
                    // clash with pid of some other writer, it is designed for 
                    // convience of reading the output of the program.
    int true_pid; // Unique pid, does not clash between readers and writers
public:
    // ReaderProcess(int & _data): data(_data){
    WriterProcess(int & _data): data(_data){
        lockPID.acquire();
            writer_pid = total_writer_count++;
            true_pid =  global_pid++;
        lockPID.release();
    }


    void entry(){
        // No thing to do 
        lockCout.acquire();
            std::cout<<"[Writer: " + std::to_string(writer_pid) +", PID: " + std::to_string(true_pid)+"] Started Writing."<<std::endl;
        lockCout.release();
        startTime =  std::chrono::high_resolution_clock::now();

    }
    void preCriticalSection(){

        // Handle the lock

        lockFIFO.acquire(); // Forces FIFO ordering on process execution

        lockResource.acquire(); // Wait till resource used by reader/writer

        lockFIFO.release(); //  Release for others


    }
    void criticalSection(){

        using namespace std::literals;
        std::this_thread::sleep_for(100ms);

        lockCout.acquire();
            std::cout<<"[Writer: " + std::to_string(writer_pid) +", PID: " + std::to_string(true_pid)+"] Value updated to: " + std::to_string(++data)<<std::endl;
        lockCout.release();

    }
    void postCriticalSection(){
        // Handle the lock

        lockResource.release(); // Release for others 
    }
    void remainder(){
        lockCout.acquire();
            std::cout<<"[Writer: " + std::to_string(writer_pid) +", PID: " + std::to_string(true_pid)+"] Finished writing."<<std::endl;
        lockCout.release();
        finishTime =  std::chrono::high_resolution_clock::now();
    }


};
int WriterProcess::total_writer_count = 0; // Stores the total no of readers that exist


using std::thread;


 
void execute( ProcessBase* process)
{
    process->entry();
    process->preCriticalSection();
    process->criticalSection();
    process->postCriticalSection();
    process->remainder();
}
int main()
{
    int data = 1;


    const int threadsToSpawn = 100;

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count()); // Random number generator


    std::vector<std::thread> threads;

    std::vector<ReaderProcess> readers; 
    readers.reserve(threadsToSpawn);

    std::vector<WriterProcess> writers;
    writers.reserve(threadsToSpawn);

    for(int i= 0 ;i <threadsToSpawn ;i++)
    {
        //generate either reader or writer randomly
        if(rng()%2 == 0)
        {

            ReaderProcess rd(data);
            readers.push_back(rd);
            threads.push_back(std::thread(execute, &readers.back()));
        }
        else
        {
            WriterProcess wd(data);
            writers.push_back(wd);
            threads.push_back(std::thread(execute, &writers.back()));

        }
    }

    // Wait till all are finish
    for(auto& thrd: threads)
        thrd.join();





    // Priting debugging information

    std::cout<<"[STATISTICS] :"<<std::endl;



    for(auto &rd:readers)
    {
        rd.turnaroundTime =  std::chrono::duration_cast<std::chrono::duration<double> >(rd.finishTime- rd.startTime).count();
    } 

    for(auto &wd:writers)
    {
        wd.turnaroundTime =  std::chrono::duration_cast<std::chrono::duration<double> >(wd.finishTime- wd.startTime).count();
    } 


    for(auto rd:readers)
        std::cout<<"[Reader: " + std::to_string(rd.reader_pid) +", PID: " + std::to_string(rd.true_pid)+"] Turnaround time "+std::to_string(rd.turnaroundTime)<<std::endl;

    for(auto wd:writers)
        std::cout<<"[Writer: " + std::to_string(wd.writer_pid) +", PID: " + std::to_string(wd.true_pid)+"] Turnaround time "+std::to_string(wd.turnaroundTime)<<std::endl;

  
    std::cout<<"As can be seen from the above output, the turnaround time is directly proportional to the arrival order, hence no starvation is here!"<<std::endl;
}
