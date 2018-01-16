#include <iostream>
#include <future>
#include <thread>
#include <atomic>
#include <algorithm>
#include <functional>
#include <vector>
using namespace std;

template <typename Iterator>
Iterator parallel_max_element(Iterator first, Iterator last, atomic<bool> &done) 
{
    try 
	{
        unsigned long const length=distance(first,last);
        unsigned long const min_per_thread=10;
 
    
 
        if(length<(2*min_per_thread)) 
		{
 
            if (first==last) return last;
            
            Iterator largest = first;
 
            while (++first!=last)
                if (*largest<*first)
                    largest=first;
            return largest;
        } else {
            Iterator const mid_point=first+(length/2);
 
            future<Iterator> async_result=async(&parallel_max_element<Iterator>,mid_point,last,ref(done));
            
            Iterator const direct_result=parallel_max_element(first,mid_point,done);
 
            Iterator direct=direct_result;
            
            Iterator asyncR=async_result.get();
 
            if(*direct>*asyncR) 
			{
                return direct;
                
            }
            
            return asyncR;
        }
        
    } catch(...) 
	{
        done=true;
        throw;
    }
}
 
template <typename Iterator>
Iterator parallel_max(Iterator first,Iterator last) 
{
    atomic<bool>done(false);
    
    return parallel_max_element(first,last,done);
    
}

int main() 
{
    vector<int>v {1,11,2,0,3,4,9,5,7,3,14,78,45,567,121,578,2345,789,123,567,234,679,288,345};
    cout<<*parallel_max(v.begin(),v.end());
}
