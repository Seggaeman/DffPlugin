#pragma once

#include "StdAfx.h"

namespace extensions {
class DffStream : public ::std::fstream
{
private:
	//do not edit this directly
	std::vector<DWORD> addresses;

public:	
	///writes a value to the stream, optional address push useful for writing section size. Returns number of bytes written
	template <typename var> inline DWORD GenericWrite(const var& input, bool pushAddress= false)
	{
		if (pushAddress)
			this->addresses.push_back(this->tellp());
		var& temp= const_cast<var&>(input);
		this->write(reinterpret_cast<char*>(&temp), sizeof(var));
		return sizeof(var);
	}

	///returns false if no addresses have been pushed, else true
	template <typename var> inline bool WriteAddressAndPop(const var& input)
	{
		if (! addresses.size())
			return false;
		//save current file pointer
		DWORD fPointer= this->tellp();
		this->seekp(addresses.back(), std::ios::beg);
		addresses.pop_back();
		var& temp= const_cast<var&>(input);
		this->write(reinterpret_cast<char*>(&temp), sizeof(input));
		this->seekp(fPointer, std::ios::beg);
		return true;
	}

	inline bool FlushSectionSize()
	{
		return this->WriteAddressAndPop<UINT>((UINT)this->tellp()-addresses.back()-8);
	}

	inline void WriteHeader(const UINT& secID, const UINT& fileVersion)
	{
		this->GenericWrite<UINT>(secID);
		this->GenericWrite<UINT>(0, true);
		this->GenericWrite<UINT>(fileVersion);
	}

	inline void writeANSIString(const std::tstring& input, const bool& stringTerminator= false, const int& reqLength= -1)
	{
#ifdef UNICODE
		std::string ansiString(input.begin(), input.end());
#else
	   const std::string& ansiString= input;
#endif
		int padding= (reqLength<0 && stringTerminator)? 1 : reqLength-ansiString.length();
		if (reqLength >0 && reqLength <= input.length())
		{
			if (stringTerminator)
				(*this) << ansiString.substr(0, reqLength-1) << (char)0;
			else
				(*this) << ansiString.substr(0, reqLength);
		}
		else
		{
			(*this) << ansiString;
		}

		for (int i=0; i < padding; ++i)
			(*this) << (char)0;
	}

	//contents of string section must be padded on 4 byte boundary
	inline void WritePaddedString(const std::tstring& inpStr, DWORD padding= 4)
	{
#ifdef UNICODE
		std::string ansiString(inpStr.begin(), inpStr.end());
#else
	   const std::string& ansiString= inpStr;
#endif
		(*this) << ansiString;
		//DWORD remainder= inpStr.length() % padding;
		for (int i =0 ; i < (padding-(inpStr.length() % padding)); ++i)
			(*this) << (char)0;
	}
};

}