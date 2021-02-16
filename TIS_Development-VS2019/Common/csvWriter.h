#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class csvWriter;

inline static csvWriter& endrow(csvWriter& file);
inline static csvWriter& flush(csvWriter& file);

class csvWriter
{
	std::ofstream _fs;
	bool _isFirst;
	const std::string _separator;
	const std::string _escapeSeq;
	const std::string _specialChar;
public:
	csvWriter(const std::string filename, const std::string separator = ",")
		: _fs()
		, _isFirst(true)
		, _separator(separator)
		, _escapeSeq("\"")
		, _specialChar("\"")
	{
		_fs.exceptions(std::ios::failbit | std::ios::badbit);
		_fs.open(filename, std::ios::out | std::ios::app);
	}

	~csvWriter()
	{
		flush();
		_fs.close();
	}

	void flush()
	{
		_fs.flush();
	}

	void endrow()
	{
		_fs << std::endl;
		_isFirst = true;
	}

	csvWriter& operator << ( csvWriter& (* val)(csvWriter&))
	{
		return val(*this);
	}

	csvWriter& operator << (const char * val)
	{
		return write(escape(val));
	}

	csvWriter& operator << (const std::string & val)
	{
		return write(escape(val));
	}

	template<typename T>
	csvWriter& operator << (const T& val)
	{
		return write(val);
	}

	//<Summary>
	// write with one or more columns of integer values, each column of data is represented by the pair <column name, column data>,
	// The dataset is represented as a vector of these columns. 
	// [Note] all columns should be the same size 
	//</Summary>
	template<typename T>
	void write(std::vector<std::pair<std::string, std::vector<T>>> dataset)
	{
		//send column names
		for(unsigned long j = 0; j < dataset.size(); ++j)
		{
			write(escape(dataset.at(j).first));
		}
		endrow();

		//send data
		for(unsigned long i = 0; i < dataset.at(0).second.size(); ++i)
		{
			for(unsigned long j = 0; j < dataset.size(); ++j)
			{
				write(dataset.at(j).second.at(i));
			}
			endrow();
		}
	}

	//<Summary>
	// write with one or more columns of values, each column of data is represented by vector of names,
	// The dataset is expected in cancatenate orders, [data1's length, data2's length]. 
	// [Note] all columns should be the same size 
	//</Summary>
	template<typename T>
	void write(std::vector<std::string> names, T* data, unsigned long long length)
	{
		if (NULL == data)
			return;

		//send column names
		for(unsigned long j = 0; j < names.size(); ++j)
		{
			write(escape(names.at(j)));
		}
		endrow();

		//send data
		T* pTgt = data;
		for(unsigned long long i = 0; i < length; ++i)
		{
			for(unsigned long j = 0; j < names.size(); ++j)
			{
				write(*(pTgt + j * length));
			}
			endrow();
			pTgt++;
		}
	}


private:
	template<typename T>
	csvWriter& write (const T& val)
	{
		if (!_isFirst)
		{
			_fs << _separator;
		}
		else
		{
			_isFirst = false;
		}
		_fs << val;
		return *this;
	}

	std::string escape(const std::string & val)
	{
		std::ostringstream result;
		result << '"';
		std::string::size_type to, from = 0u, len = val.length();
		while (from < len &&
			std::string::npos != (to = val.find_first_of(_specialChar, from)))
		{
			result << val.substr(from, to - from) << _escapeSeq << val[to];
			from = to + 1;
		}
		result << val.substr(from) << '"';
		return result.str();
	}
};

inline static csvWriter& endrow(csvWriter& file)
{
	file.endrow();
	return file;
}

inline static csvWriter& flush(csvWriter& file)
{
	file.flush();
	return file;
}
