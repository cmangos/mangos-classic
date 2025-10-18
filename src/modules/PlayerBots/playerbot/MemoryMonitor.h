#ifndef _MemoryMonitor_H
#define _MemoryMonitor_H
//#define MEMORY_MONITOR

#include <string>
#include <unordered_map>
#include <map>
#include <thread>
#include <list>

class MemoryMonitor
{
    public:
        MemoryMonitor() {};
        virtual ~MemoryMonitor() {};
        static MemoryMonitor& instance()
        {
            static MemoryMonitor instance;
            return instance;
        }

	public:        
        //Objecttype is the name of the class/struct.
        //Object is a pointer to the object or unique identifier.
        //level = 0 only count total number of objects, otherwise log the creation stack of any (object%level) = 0 object.
        void Add(std::string objectType, uint64_t object, int level = 0, std::string stack = ""); //Method to call when object gets created.
        void Rem(std::string objectType, uint64_t object, int level = 0); //Method to call when object gets destroyed.
        void Print(); //Print number of objects that still exist and their creation call-stack.
        void Browse();
        void LogCount(std::string filename);
	private:
        std::unordered_map<std::thread::id, std::map<std::string, int>> objectnumbers;
        std::list<std::map<std::string, int >> objectnumbersHist;
        std::unordered_map < std::thread::id, std::unordered_map<std::string, std::unordered_map<uint64_t,std::pair<std::string,time_t>>>> adds, rems;
        bool headers = false;
};


#define sMemoryMonitor MemoryMonitor::instance()

#endif
