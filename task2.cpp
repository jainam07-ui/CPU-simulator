#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<queue>
using namespace std;

struct Task{
    string id;
    int burst_time;
    vector<string> mem;
    int mem_idx=0;
};

vector<Task> parseInputFile(string filename){
    vector<Task> task_list;
    ifstream infile(filename);
    if(!infile.is_open()){
        cerr << "Error opening file!" << endl;
        return task_list;
    }
    string line;
    while(getline(infile,line)){
        stringstream ss(line);
        Task temp_task;
        string discard;
        ss >> discard;
        ss >> temp_task.id;
        ss >> discard;
        ss >> temp_task.burst_time;
        ss >> discard;
        string mem_block;
        while(ss >> mem_block){
            temp_task.mem.push_back(mem_block);
        }
        task_list.push_back(temp_task);
    }
    infile.close();
    return task_list;
}

struct CacheLevel{
    string lev_id;
    int capacity;
    int latency;
    vector<string> slots;
};

class Cache{
    private:
    CacheLevel L1;
    CacheLevel L2;
    CacheLevel L3;
    int ram_access=0;

    void runWaterfall(string block) {
        cout << "Promoting " << block << " to L1..." << endl;
        string ch = insert(L1, block);
        
        if (ch != "00") {
            cout << "   -> L1 full! Evicting " << ch << " down to L2..." << endl;
            ch = insert(L2, ch);
        }
        if (ch != "00") {
            cout << "   -> L2 full! Evicting " << ch << " down to L3..." << endl;
            ch = insert(L3, ch);
        }
        if (ch != "00") {
            cout << "   -> L3 full! " << ch << " permanently evicted from cache hierarchy." << endl;
        }
    }

    public:
    Cache(){
        L1 = {"L1", 1, 4, {}};
        L2 = {"L2", 4, 12, {}};
        L3 = {"L3", 16, 40, {}};
    }
    
    void printcache(CacheLevel &level){
        cout << level.lev_id << ": [ ";
        for(int i=0; i<level.slots.size(); i++){
            cout << level.slots[i] << " ";
        }
        cout << "]";
    }
    
    string insert(CacheLevel &level, string block){
        string evicted_block = "00";
        
        for(int i=0; i<level.slots.size(); i++){
            if(level.slots[i] == block){
                return "00";
            }
        }
        
        if(level.slots.size() >= level.capacity){
            evicted_block = level.slots[0]; 
            level.slots.erase(level.slots.begin());
        }
        
        level.slots.push_back(block);
        return evicted_block;
    }
    
    int access(string block){
        printcache(L1);
        for(int i=0; i<L1.slots.size(); i++){
            if(L1.slots[i] == block){
                cout<< " >> Hit" << endl;
                return L1.latency;
            }
        }
        cout << " >> Miss" << endl;
        
        printcache(L2);
        for(int i=0; i<L2.slots.size(); i++){
            if(L2.slots[i] == block){
                cout << " >> Hit" << endl;
                runWaterfall(block);
                return L2.latency;
            }
        }
        cout << " >> Miss" << endl;
        
        printcache(L3);
        for(int i=0; i<L3.slots.size(); i++){
            if(L3.slots[i] == block){
                cout << " >> Hit" << endl;
                runWaterfall(block);
                return L3.latency;
            }
        }
        cout << " >> Miss" << endl;
        
        cout << "Accessing RAM for " << block << "..." << endl;
        ram_access++;
        runWaterfall(block);
        return 200;
    }
    
    void printstate(){
        cout << "Current State -> ";
        printcache(L1); cout << " | ";
        printcache(L2); cout << " | ";
        printcache(L3); cout << endl;
        cout << "--------------------------------------------------------" << endl;
    }
    
    int getramaccess(){
        return ram_access;
    }
};

int main(){
    vector<Task> loaded_tasks = parseInputFile("input_task2.txt");
    queue<Task> ready_queue;
    for(const auto& task : loaded_tasks){
        ready_queue.push(task);
    }
    int global_cycle = 1;
    int quantum = 2;
    Cache cache;
    while(!ready_queue.empty()){
        Task curr_task = ready_queue.front();
        ready_queue.pop();
        int time_slice = 0;
        while(time_slice < quantum && curr_task.burst_time > 0){
            cout << "Cycle " << global_cycle << " - Running: " << curr_task.id;
            if(curr_task.mem_idx < curr_task.mem.size()){
                string req_block = curr_task.mem[curr_task.mem_idx];
                cout << " | Requesting: " << req_block << endl;
                int latency_paid = cache.access(req_block);
                curr_task.mem_idx++;
            }
            else{
                cout << " | Requesting: None" << endl;
            }
            cache.printstate();
            time_slice++;
            global_cycle++;
            curr_task.burst_time--;
        }
        if(curr_task.burst_time > 0){
            ready_queue.push(curr_task);
        }
        else{
            cout << ">>> Task " << curr_task.id << " completed. <<<" << endl;
        }
    }
    cout << "\n=== Final Results ===" << endl;
    cout << "Total Cycles: " << global_cycle - 1 << endl;
    cout << "Tasks Completed: " << loaded_tasks.size() << endl;
    cout << "Scheduler: Round Robin (Quantum = 2)" << endl;
    cout << "Total RAM Accesses: " << cache.getramaccess() << endl;
    return 0;
}