#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <climits>
#include <algorithm>
#include <queue>

using namespace std;



void FCFS(vector<int> pid, vector<int> arrival_time, vector<int> burst_time);
void SJF(vector<int> pid, vector<int> arrival_time, vector<int> burst_time);
void RR(vector<int> pid, vector<int> arrival_time, vector<int> burst_time, int time_quantum);

int main(int argc, char* argv[]){
    /// Check for 2 arguments 
    if (argc < 3) {
    cerr << "Usage: " << argv[0]
         << " <input_file> <FCFS|SJF|RR> [time_quantum]" << endl;
    return 1;
    }

   
    /// Check if the input file can be opened
    ifstream inputFile(argv[1]);
    if(!inputFile.is_open()){
        cerr << "Error opening file: " << argv[1] << endl;
        return 0;
    }

    /// Declare vectors to hold the data
    string line;
    vector<int> pid;
    vector<int> arrival_time;
    vector<int> burst_time;

    /// Skip the first line (header) of the CSV file
    getline(inputFile, line); 

    /// Read each line of the CSV file and parse the values
    while(getline(inputFile, line)){
        stringstream ss(line);
        string cell;
        vector<string> row;

        // Get each cell in the whitespace-separated row
        while(ss >> cell){
            row.push_back(cell);
        }

        /// Verify that the row has exactly 3 elements (pid, arrival_time, burst_time)
        if(row.size() != 3){
            cerr << "Invalid row format: " << line << endl;
            continue; // Skip this row and continue with the next
        }

        /// Push back the values into the respective vectors
        pid.push_back(stoi(row[0]));
        arrival_time.push_back(stoi(row[1]));
        burst_time.push_back(stoi(row[2]));
    }

    /// Determine which scheduling algorithm to use based on the command line argument
    string algorithm = argv[2];
    if(algorithm == "FCFS"){
        FCFS(pid, arrival_time, burst_time);
    } else if(algorithm == "SJF"){
        SJF(pid, arrival_time, burst_time);
    } else if(algorithm == "RR"){
        if(argc < 4){
            cerr << "Please provide a time quantum for the Round Robin scheduling algorithm." << endl;
            return 0;
        }
        int time_quantum = stoi(argv[3]);
        RR(pid, arrival_time, burst_time, time_quantum);
    } else {
        cerr << "Invalid scheduling algorithm specified. Please use FCFS, SJF, or RR." << endl;
    }
    
    inputFile.close();
}

/// Function to implement the First-Come, First-Served (FCFS) scheduling algorithm
void FCFS(vector<int> pid, vector<int> arrival_time, vector<int> burst_time) {
    /// Declare variables to hold the start time, end time, running time, and waiting time for each process
    int start_time = 0;
    int end_time = 0;
    int running_time = 0;
    int waiting_time = 0;
    float average_waiting_time = 0;

    /// Loop through each process and calculate the start time, end time, running time, and waiting time
    for(size_t i = 0; i < pid.size(); i ++){
        /// Keep track of first process start time
        if(pid[i] == 0){
            if(arrival_time[i] > 0){
                printf("Idle from %d to %d\n", end_time, arrival_time[i]);
            }
            start_time = arrival_time[i];
        } else {
            start_time = end_time;
            if(arrival_time[i] > start_time){
                printf("Idle from %d to %d\n", start_time, arrival_time[i]);
                start_time = arrival_time[i];
            }
        }

        /// Calculate end time, running time, and waiting time, and increment waiting time for average calculation
        end_time = start_time + burst_time[i];
        running_time = burst_time[i];
        waiting_time = start_time - arrival_time[i];
        average_waiting_time += waiting_time;

        /// Print the results for each process
        printf("Process %d: Arrival Time = %d, Start Time = %d, End Time = %d, Running Time = %d, Waiting Time = %d\n", 
            pid[i], arrival_time[i], start_time, end_time, running_time, waiting_time);
    }
    average_waiting_time /= pid.size();
    cout << "Average Waiting Time = " << average_waiting_time << endl;
}

/// Function to implement the Shortest Job First (SJF) scheduling algorithm
void SJF(vector<int> pid, vector<int> arrival_time, vector<int> burst_time) {
    /// Create a vector to hold the order of jobs based on SJF scheduling
    vector<int> job_order;
    int current_time = 0;

    float average_waiting_time = 0;
    while(!burst_time.empty()){
        int shortest_burst = INT_MAX;
        size_t shortest_index = burst_time.size();

        /// Find the process with the shortest burst time that has arrived by the current time
        for(size_t i = 0; i < burst_time.size(); i++){
            if(arrival_time[i] <= current_time &&
               burst_time[i] < shortest_burst){
                shortest_burst = burst_time[i];
                shortest_index = i;
            }
        }

        /// If no process has arrived, advance to the next arrival time.
        if(shortest_index == burst_time.size()){
            int previous_time = current_time;
            current_time = arrival_time[0];
            for(size_t i = 1; i < arrival_time.size(); i++){
                if(arrival_time[i] < current_time){
                    current_time = arrival_time[i];
                }
            }
            printf("Idle from %d to %d\n", previous_time, current_time);
            continue;
        }

        job_order.push_back(pid[shortest_index]);
        current_time += burst_time[shortest_index];

        /// Calculate and print the waiting time for each process
        int waiting_time = current_time - burst_time[shortest_index] - arrival_time[shortest_index];
        printf("Process %d: Arrival Time = %d, Start Time = %d, End Time = %d, Running Time = %d, Waiting Time = %d\n", 
            pid[shortest_index], arrival_time[shortest_index], current_time - burst_time[shortest_index], current_time, burst_time[shortest_index], waiting_time);   


        /// Erase the selected job from all parallel vectors at the same index.
        burst_time.erase(burst_time.begin() + shortest_index);
        pid.erase(pid.begin() + shortest_index);
        arrival_time.erase(arrival_time.begin() + shortest_index);
        average_waiting_time += waiting_time;
    }
    /// Print average waiting time for all processes after the SJF scheduling is complete
    average_waiting_time /= job_order.size();
    cout << "Average Waiting Time = " << average_waiting_time << endl;
}

void RR(vector<int> pid, vector<int> arrival_time, vector<int> burst_time, int time_quantum) {
    /// Variable declarations for Round Robin scheduling
    int current_time = 0;
    float average_waiting_time = 0;
    vector<int> completion_time(pid.size());
    vector<int> waiting_times(pid.size());

    /// Remaining burst time vector to track the remaining burst time for each process
    vector<int> remaining_burst_time = burst_time; // Create a copy of burst_time to track remaining time

    queue<size_t> ready_queue;
    vector<bool> added_to_queue(pid.size(), false);
    size_t completed_processes = 0;

    /// Loop through the ready queue until all processes are completed
    while(completed_processes < pid.size()){
        /// Add all processes that have arrived to the ready queue
        for(size_t i = 0; i < pid.size(); i++){
            if(!added_to_queue[i] && arrival_time[i] <= current_time){
                ready_queue.push(i);
                added_to_queue[i] = true;
            }
        }

        /// If no process is ready, advance to the next arrival time
        if(ready_queue.empty()){
            int next_arrival = INT_MAX;
            for(size_t i = 0; i < pid.size(); i++){
                if(!added_to_queue[i] && arrival_time[i] < next_arrival){
                    next_arrival = arrival_time[i];
                }
            }
            printf("Idle from %d to %d\n", current_time, next_arrival);
            current_time = next_arrival;
            continue;
        }

        /// Select the process at the front of the ready queue
        size_t process_index = ready_queue.front();
        ready_queue.pop();
        int start_time = current_time;
        int running_time = min(time_quantum, remaining_burst_time[process_index]);
        current_time += running_time;
        remaining_burst_time[process_index] -= running_time;

        /// Add processes that arrived while the selected process was running
        for(size_t i = 0; i < pid.size(); i++){
            if(!added_to_queue[i] && arrival_time[i] <= current_time){
                ready_queue.push(i);
                added_to_queue[i] = true;
            }
        }

        /// If the process is not complete, place it at the back of the ready queue
        if(remaining_burst_time[process_index] > 0){
            ready_queue.push(process_index);
        } else {
            completion_time[process_index] = current_time;
            waiting_times[process_index] =
                completion_time[process_index] - arrival_time[process_index] - burst_time[process_index];
            average_waiting_time += waiting_times[process_index];
            completed_processes++;
        }

        /// Print the results for the current process
        printf("Process %d: Start Time = %d, End Time = %d, Running Time = %d\n",
            pid[process_index], start_time, current_time, running_time);
    }

    cout << "\n";
    /// print final PID, Arrival Time, Running Time, End times, and waiting times for each process
    for(size_t i = 0; i < pid.size(); i++){
        printf("Process %d: Arrival Time = %d, Running Time = %d, End Time = %d, Waiting Time = %d\n", 
            pid[i], arrival_time[i], burst_time[i],
            completion_time[i], waiting_times[i]);
    }
    average_waiting_time /= pid.size();
    cout << "Average Waiting Time = " << average_waiting_time << endl;  

        
}