#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <initializer_list>



/*
  This Header File implements an easy way to display logging information for an application.
  Logging can be selectively turned on or off by using characters as identifiers for which log calls should be executed.

  LOG::write(const std::string& message, std::ostream& os)
      Whenever this function is called, it writes a string of the form [hh:mm:ss] -> message to the output stream os.
  
  LOG::write(const std::string& message, std::ostream& os, char identifier OR const std::initializer_list<char>& identifiers)
      This function writes the message together with a time stamp to os, if the provided identifiers are currently activated.
	  This provides a way to selectively activate or deactivate logging.
	  As an example, 'm' could stand for logging calls associated with memory allocation.

  LOG::activate(char identifier)
      Activate the logging of messages using the given identifier

  LOG::deactivate(char identifier)
      Deactivate the logging of messages using the given identifier
*/



class LOG {
	//This singleton class provides thread safe information on which logging identifiers are active at the moment
	class Log
	{
	public:
		static Log& instance();	//Get the Log instance (Return the existing one or else construct a new one)

		bool is_active(char identifier) const;	//Provide thread safe access to check if a logging identifier is currently active
		void activate(char identifier);		//Activate the logging of information using this identifier
		void deactivate(char identifier);	//Deactivate the logging of information using this identifier
		bool log(const std::string& message, std::ostream& os) const;	//Write the message to the stream (use the mutex m to make it thread safe)
		bool log(const std::string& message, std::ostream& os, char identifier) const;

	private:
		static Log* inst;		//A pointer to an instance of Log
		bool active[256];		//Stores the currently selected identifiers to log
		static std::mutex m;	//Make the Log instance thread safe to allow logging of multiple threads

		Log();	//Constructor is private as this class is a singleton
	};


public:
	static void write(const std::string& message, std::ostream& os);
	static void write(const std::string& message, std::ostream& os, char identifier);
	static void write(const std::string& message, std::ostream& os, const std::initializer_list<char>& identifiers);
	static void activate(char identifier);
	static void deactivate(char identifier);
};

LOG::Log* LOG::Log::inst = nullptr;
std::mutex LOG::Log::m;



//---------- LOG::Log Implementation ----------//

inline LOG::Log& LOG::Log::instance()
{
	std::lock_guard<std::mutex> guard(m);
	if (!inst)	//Construct a new instance of Log, if none exists yet
		inst = new Log();
	return *inst;
}


inline bool LOG::Log::is_active(char identifier) const
{
	std::lock_guard<std::mutex> guard(m);
	return active[identifier];
}


inline void LOG::Log::activate(char identifier)
{
	std::lock_guard<std::mutex> guard(m);
	active[identifier] = true;
}


inline void LOG::Log::deactivate(char identifier)
{
	std::lock_guard<std::mutex> guard(m);
	active[identifier] = false;
}


LOG::Log::Log()
{
	//Fill the active identifiers array with false
	for (bool& f : active)
		f = false;
}


inline bool LOG::Log::log(const std::string& message, std::ostream& os) const
{
	std::lock_guard<std::mutex> guard(m);
	std::time_t t = std::time(nullptr);
	std::tm tm;
	localtime_s(&tm, &t);
	os << "[" << std::put_time(&tm, "%T") << "] -> " << message << std::endl;
	return true;
}


inline bool LOG::Log::log(const std::string& message, std::ostream& os, char identifier) const
{
	std::lock_guard<std::mutex> guard(m);
	if (active[identifier]) {
		std::time_t t = std::time(nullptr);
		std::tm tm;
		localtime_s(&tm, &t);
		os << "[" << std::put_time(&tm, "%T") << "] -> " << message << std::endl;
		return true;
	}
	return false;
}



//---------- Logging Functions ----------//

inline void LOG::write(const std::string& message, std::ostream& os)
{
	Log::instance().log(message, os);
}


inline void LOG::write(const std::string& message, std::ostream& os, char identifier)
{
	Log::instance().log(message, os, identifier);
}


inline void LOG::write(const std::string& message, std::ostream& os, const std::initializer_list<char>& identifiers)
{
	for (char i : identifiers) {
		if (Log::instance().log(message, os, i))
			break;
	}
}


inline void LOG::activate(char identifier)
{
	Log::instance().activate(identifier);
}


void LOG::deactivate(char identifier)
{
	Log::instance().deactivate(identifier);
}