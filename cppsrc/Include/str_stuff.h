#pragma once
#include <string>
#include <vector>

using std::vector;

std::string & ltrim(std::string & str)
{
  auto it2 =  std::find_if( str.begin() , str.end() , [](char ch){ return !std::isspace(ch) ; } );
  str.erase( str.begin() , it2);
  return str;
}

std::string & rtrim(std::string & str)
{
  auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace(ch) ; } );
  str.erase( it1.base() , str.end() );
  return str;
}

std::string& trim(std::string& str)
{
   return ltrim(rtrim(str));
}

// Create a vector of strings from a single string delimited with
// the character in the supplied parameter: delim.
vector<std::string> tokenize(const std::string &s, const char delim)
{
  vector<std::string> out;
  std::string::size_type beg = 0;
  for (std::string::size_type end = 0; (end = s.find(delim, end)) != std::string::npos; ++end)
  {
    out.push_back(s.substr(beg, end - beg));
    beg = end + 1;
  }

  auto rs = s.substr(beg);
  out.push_back(trim(rs));
  return out;
}
