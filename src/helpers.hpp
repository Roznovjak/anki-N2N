#ifndef Helpers_h
#define Helpers_h

#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <exception>

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <unicode/normlzr.h>


using json = nlohmann::json;
extern const char* AnkiConnect;
extern const int num_of_required_program_args;


class my_exception : public std::exception
{
private:
   std::string msg;
public:
   enum exception_class
   {
      GENERAL,
      ANKI,
      ICU,
      CURL,
   };
   
   my_exception(std::string error_msg, const exception_class e_class = GENERAL);
   const char* what() const noexcept { return msg.c_str();}
};


class json_getter
{
public:
   static void get_deck_names(json& request);
   static void get_note_types(json& request);
   static void get_note(json& note, const std::string& deck_name, const std::string& note_type, const std::string& front, const std::string& back, const std::vector<std::string>& tags);
   static void get_can_add_notes(json& request, const std::string& deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags);
   static void get_add_notes( json& request, const std::string& deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags);
   
   json_getter()=delete;
   json_getter(const json_getter&)=delete;
   json_getter(json_getter&&)=delete;
};


size_t curl_write_cb(void* buffer, size_t size, size_t nmemb, void* userp);
std::string trim_string(const std::string& str, const std::string& trim = " ");
std::string to_lowercase( const std::string& str );
std::string normalize_string(const char* str);

bool parse_line( const std::string& line, std::string& key, std::string& value);
void process_file( const char* file_path, std::unordered_map<std::string, std::string>& dict);

CURLcode send_request(CURL* curl, const json& request, std::string& response );
void test_ankiconnect_response( const json& response );
void test_deck_name(CURL* curl, const std::string& deck_name);
void test_note_type(CURL* curl, const std::string& note_type);
void test_can_add_notes(CURL* curl, const std::string& deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags);
void add_notes(CURL* curl, const std::string& deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags);

#endif /* Helpers_h */
