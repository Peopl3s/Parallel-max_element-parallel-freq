#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <thread>
#include <iterator>
#include <fstream>
#include <vector>
#include <future>
 
using namespace std;
 
class join_threads
{
    vector<thread>& threads;
    
    public:
        
    explicit join_threads(vector<thread>&threads_):
        threads(threads_){}
        
    ~join_threads()
    {
        for(int i=0;i<threads.size();i++)
        {
            if(threads[i].joinable())
            {
                threads[i].join();
            }
        }
    }
        
        
};
 
 
map<char,int> FoC(string::iterator beg,string::iterator en)
 {
    map<char,int>m {};
    
    for(auto it=beg; it!=en; it++) 
    {
        ++m[*it];
    }
    
    return m;
}
 
map<char,int>& operator+=(map<char,int>&a, const map<char,int> &b) 
{
 
    for(map<char,int>::const_iterator it=b.begin(); it!=b.end(); it++) 
    {
        if(a.find(it->first)!=a.end()) 
        {
            a[it->first]+=b.at(it->first);
            
        } else a[it->first]+=it->second;
    }
    
    return a;
}
 
const map<char,int> operator+(const map<char,int>&a, const map<char,int> &b)
{
    map<char,int> res{a};
    
    res+=b;
    
    return res;
}
 
 
 
template<typename Iterator>
map<char,int> parrallel_FoC(Iterator first,Iterator last)
{
    unsigned long const length=distance(first,last);
    
    if(!length)
    return map<char,int>{};
    
    unsigned long const min_per_thread=25;
    unsigned long const max_threads=(length+min_per_thread-1)/min_per_thread;
    
    unsigned long const hardware_threads=thread::hardware_concurrency();
    
    unsigned long const num_threads=min(hardware_threads!=0?hardware_threads:2,max_threads);
    
    unsigned long const block_size=length/num_threads;
    
    vector<future<map<char,int>>> futures(num_threads-1);
    vector<thread>threads(num_threads-1);
    join_threads joiner(threads);
    
    Iterator block_start=first;
    
    for(int i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        advance(block_end,block_size);
        packaged_task<map<char,int>(string::iterator,string::iterator)>task(FoC);
    
        
        futures[i]=task.get_future();
        threads[i]=thread(move(task),block_start,block_end);
        block_start=block_end;
    }
    
    map<char,int>res=FoC(block_start,last);
    
    for(int i=0;i<(num_threads-1);++i)
    {
        res+=futures[i].get();
    }
    return res;
    
}
 
 
 
int main() {
    
    ifstream ff("t.txt");
    
    string s {istream_iterator<char>(ff),istream_iterator<char>()};
    
    map<char,int>r=parrallel_FoC(s.begin(),s.end());
    
    for(map<char,int>::iterator it=r.begin(); it!=r.end(); it++) 
    {
        cout<<it->first<<" "<<it->second<<endl;
    }
    
    return 0;
}
