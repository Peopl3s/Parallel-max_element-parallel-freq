#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <thread>
#include <iterator>
#include <fstream>
#include <vector>
#include <future>

namespace tools_
{
	class join_threads
	{
		std::vector<std::thread> &threads;

	public:

		explicit join_threads(std::vector<std::thread> &threads_):
			threads(threads_) {}

		~join_threads() 
		{
			for(int i = 0; i < threads.size(); ++i) 
			{
				if(threads[i].joinable())
				 {
					threads[i].join();
				}
			}
		}
};

void clearPunct(std::string &str)
{
	str.erase(remove_if(str.begin(), str.end(), [](const char &ch) {
		return ispunct(ch);
	}), str.end());
}

void toLowerCase(std::string &str)
{
	transform(str.begin(), str.end(), str.begin(), [](const char &ch) {
		return tolower(ch);
	});
}

std::string canonicalForm(const std::string &oldWord)
{
	std::string newWord {oldWord};
	if(newWord.find_first_of(".,-[]()!?:;'\"") != std::string::npos)
	{
		clearPunct(newWord);
	}
	toLowerCase(newWord);
	
	return newWord;
}

void removeDashSetLenTwo(std::vector<std::string> &vec, bool lenTwo=false) 
{
	if(lenTwo) 
	{
		vec.erase(remove_if(vec.begin(), vec.end(), [](const std::string &str) {
			return str.length() < 2 && str != "ÿ" && str != "i";
		}), vec.end());
	}

	vec.erase(remove_if(vec.begin(), vec.end(), [](const std::string &str) {
		return str == "—";
	}), vec.end());

}

template<typename Key = std::string, typename Value = unsigned int>
void frequencyWriteFile(const std::vector<std::pair<Key, Value>> &vec)
{
	std::ofstream file("frequency.txt", std::ios_base::out);
	if(file.is_open())
	{
		for(const auto &pairs:vec)
		{
			file << pairs.first + " " + std::to_string(pairs.second) + "\n";
		}
	} 
	else 
	{
		std::cerr << "Opening file error" << std::endl;
	}
}

}


template<typename Key = std::string, typename Value = unsigned int>
std::map<Key, Value> freqC(std::vector<std::string>::iterator beg, std::vector<std::string>::iterator en) 
{
	std::map<Key, Value> m {};

	for(auto it = beg; it != en; ++it)
	{
		++m[*it];
	}

	return m;
}

template<typename Key = std::string, typename Value = unsigned int>
std::map<Key, Value>& operator+=(std::map<Key, Value> &a, const std::map<Key, Value> &b) 
{

	for(typename std::map<Key, Value>::const_iterator it = b.begin(); it != b.end(); ++it) 
	{
		if(a.find(it->first) != a.end())
		{
			a[it->first] += b.at(it->first);

		} else a[it->first] += it->second;
	}

	return a;
}

template<typename Key = std::string, typename Value = unsigned int>
const std::map<Key, Value> operator+(const std::map<Key, Value> &a, const std::map<Key, Value> &b) 
{
	std::map<Key, Value> res {a};
	res += b;	
	return res;
}

template<typename Iterator, typename Key = std::string, typename Value = unsigned int>
std::map<Key, Value> parrallel_freqC(Iterator first, Iterator last) 
{
	unsigned long const length = distance(first, last);

	if(!length)
		return std::map<Key, Value> {};

	unsigned long const min_per_thread = 25;
	unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;

	unsigned long const hardware_threads = std::thread::hardware_concurrency();

	unsigned long const num_threads = std::min(hardware_threads != 0? hardware_threads : 2, max_threads);

	unsigned long const block_size = length / num_threads;

	std::vector<std::future<std::map<Key, Value>>> futures(num_threads-1);
	std::vector<std::thread>threads(num_threads - 1);
	tools_::join_threads joiner(threads);

	Iterator block_start = first;

	for(int i = 0; i < (num_threads - 1); ++i)
	{
		Iterator block_end = block_start;
		advance(block_end, block_size);
		std::packaged_task<std::map<Key, Value>(std::vector<std::string>::iterator, std::vector<std::string>::iterator)>task(freqC<>);

		futures[i] = task.get_future();
		threads[i] = std::thread(move(task),block_start,block_end);
		block_start = block_end;
	}

	std::map<Key, Value> res = freqC(block_start, last);

	for(int i = 0; i < (num_threads - 1); ++i)
	{
		res += futures[i].get();
	}
		
	return res;
}

int main()
{
	setlocale(LC_ALL, "RUS");
	std::string path {};
	std::cin >> path;
	std::ifstream file(path, std::ios_base::in);
	if(file.is_open())
	{
		std::vector<std::string>vec {};
		transform(std::istream_iterator<std::string> {file}, std::istream_iterator<std::string> {}, back_inserter(vec), [](const std::string &s) {
			return tools_::canonicalForm(s);
		});
		tools_::removeDashSetLenTwo(vec, true);
		
        //auto start_time = std::chrono::steady_clock::now();
        
		std::map<std::string, unsigned int> res = parrallel_freqC(vec.begin(), vec.end());
		
        //auto end_time = std::chrono::steady_clock::now();
        //auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        //std::cout << elapsed_ms.count() << "ms\n";
        
		std::vector<std::pair<std::string, unsigned int>> vecPair {res.begin(), res.end()};
		sort(vecPair.begin(), vecPair.end(), [](const auto &p1, const auto &p2) {
			return p1.second > p2.second;
		});
		tools_::frequencyWriteFile(vecPair);

		for(auto it = vecPair.cbegin(); it != vecPair.cbegin() + 10; ++it)
		{
			std::cout << it -> first << " " << it -> second<< std::endl;
		}

	} else std::cerr << "Opening file error";

}

//parrallel   106 ms. (1mb) | 1781 ms.  (file txt 52 mb)   |  6 538 ms. (file txt 208 mb)       |  22 488 (file txt 500 Mb)
// stock     80 ms.   (1mb) | 5 700 ms. (file txt 52 mb)   |  23 302 ms. (file txt 208 mb)      |  64 472 (file txt 500 Mb)
