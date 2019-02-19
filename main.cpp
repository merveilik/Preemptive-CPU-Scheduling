
#include <iostream>
#include <queue>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>

using namespace std;

// All instructions have a name and an execution time
struct Instruction {
    string name;
    int time;

    Instruction(string _name, int _time){
        name = _name;
        time = _time;
    }
};

//All processes have a name, priority, code file and arrival time
struct Process {
    string name;
    int priority;
    string code_file;
    int arrival_time;
    vector <Instruction> instructions;
    int last_line;
    int turnaround_time;
    int waiting_time;
    int total_instruction_time;
    Process(string _name, int _priority, string _codefile, int _arrivaltime){
        name = _name;
        priority = _priority;
        code_file = _codefile;
        arrival_time = _arrivaltime;
        last_line = 0;
        total_instruction_time = 0;
        readInstructions();
    }

    /*
     * This method is used for read the code file of
     * a process which includes information about instructions  */
    void readInstructions() {
        string ins_name;
        int ins_time;
        ifstream in(code_file+".txt");
        while (in >> ins_name >> ins_time) {
            Instruction ins(ins_name, ins_time);
            total_instruction_time += ins_time;
            instructions.push_back(ins);
        }
        in.close();
    }
    bool isFinished() {           //Check if process is done
        return last_line == instructions.size();
    }
};

/*
 * A comparator used for sort processes according to their priorities
 */
struct comparePriority{
public:
    bool operator()(Process const &p1, Process const &p2){

        if(p1.priority == p2.priority)
            return p1.arrival_time>p2.arrival_time;
        return p1.priority>p2.priority;
    }
};

/*
 * A comparator used for sort processes according to their arrival times
 */
struct compareArrivalTime{
public:
    bool operator()(Process const &p1, Process const &p2){
        return p1.arrival_time>p2.arrival_time;
    }
};

/*
 * Prints current status of ready queue with processes and their executing lines in it
 */
void printReadyQueue(ofstream& myfile, priority_queue<Process,vector<Process>,comparePriority> q, int time) {
    myfile << time;
    myfile << ":HEAD-";
    while (!q.empty()) {
        if(q.size() > 1)
            myfile << q.top().name << "[" << q.top().last_line+1 << "]" <<"-";
        else
            myfile << q.top().name << "[" << q.top().last_line+1 << "]";
        q.pop();
    }
    myfile << "-TAIL" << endl;
}

int main(int argc, char *argv[]) {


    priority_queue<Process,vector<Process>,comparePriority> ready_queue; //Ready processes are sorted there
    priority_queue<Process,vector<Process>,compareArrivalTime> job_queue; //All processes are stored there
    priority_queue<Process,vector<Process>,compareArrivalTime> times_for_output; //A queue to hold waiting and turnaround times of processes

    int time = 0;  //Current time is 0
    ifstream infile(argv[1]);
    ofstream myfile(argv[2]);

    string p_name, p_file;
    int p_time;
    int p_priority;

    while (infile >> p_name >> p_priority >> p_file >> p_time) {
        Process p(p_name, p_priority, p_file, p_time);
        job_queue.push(p);
    }

    infile.close();
//Program will continue if there is any process waiting in any queue
    while (!ready_queue.empty() || !job_queue.empty()) {

    while(!job_queue.empty() && job_queue.top().arrival_time <= time){ //Put all ready processes to ready queue
        ready_queue.push(job_queue.top());
        job_queue.pop();
    }

        printReadyQueue(myfile,ready_queue, time); //Print current ready queue
        if(!ready_queue.empty()) {
            Process processOnCPU = ready_queue.top(); //Assign CPU to process with highest priority from ready queue
            ready_queue.pop();
            bool newProcessCame = false;
            while(!processOnCPU.isFinished() && !newProcessCame){ //Continue execution until it is finished and no new process has arrived
                Instruction ins = processOnCPU.instructions[processOnCPU.last_line];
                time += ins.time;  //Update current time
                processOnCPU.last_line++;  //Pass to next instruction line of that process

                while (!job_queue.empty() && job_queue.top().arrival_time <= time) { //If any process became ready while execution
                   newProcessCame=true;                                              //put them into ready queue and make boolean true
                    ready_queue.push(job_queue.top());
                    job_queue.pop();
                }
            }

            if(processOnCPU.isFinished()){  //Calculate turnaround and waiting times of finished process
                processOnCPU.turnaround_time = time - processOnCPU.arrival_time;
                processOnCPU.waiting_time = time - processOnCPU.total_instruction_time - processOnCPU.arrival_time;
                times_for_output.push(processOnCPU);  //put time values in the queue
            }
            else{  //Put unfinished process in the ready queue again
                ready_queue.push(processOnCPU);
            }
        }
        else if(!job_queue.empty()){
            time = job_queue.top().arrival_time;  //Update current time
        }
    }
    //All processes are done. Print last (empty) status of ready queue
    myfile << time;
    myfile << ":HEAD-";
    myfile << "-TAIL" << endl;
    myfile << endl;

    //Traverse times_for_output queue and print turnaround and waiting times of processes
while(!times_for_output.empty()) {
    Process p = times_for_output.top();
    myfile << "Turnaround time for " << p.name << " = " << p.turnaround_time << " ms" << endl;
    myfile << "Waiting time for " << p.name << " = " << p.waiting_time << endl;
    times_for_output.pop();
}

    return 0;
}