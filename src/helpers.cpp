#include "helpers.hpp"

const char* AnkiConnect{ "http://127.0.0.1:8765" };
const int num_of_required_program_args{ 4 };

my_exception::my_exception(std::string error_msg, const exception_class e_class) : msg{error_msg}
{
   switch (e_class)
   {
      case ANKI:
         msg.insert(0,"Anki-Connect Error: ");
         break;
      case ICU:
         msg.insert(0,"ICU Error: ");
         break;
      case CURL:
         msg.insert(0,"CURL Error: ");
         break;
      case GENERAL:
         break;   // suppress warning msg
   }
}


void json_getter::get_deck_names(json& request)
{
   request["action"] = "deckNames";
   request["version"] = 6;
}

void json_getter::get_note_types(json& request)
{
   request["action"] = "modelNames";
   request["version"] = 6;
}

void json_getter:: get_note(json& note,const std::string& deck_name, const std::string& note_type, const std::string& front, const std::string& back, const std::vector<std::string>& tags)
{
   note["deckName"] = deck_name;
   note["modelName"] = note_type;
   note["fields"]["Front"] = front;
   note["fields"]["Back"] = back;
   note["tags"] = json::array();
   for( const auto& elem : tags )
      note.at("tags").push_back(elem);
}

void json_getter:: get_can_add_notes(json& request,const std::string & deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags)
{
   request["action"] = "canAddNotes";
   request["version"] = 6;
   request["params"]["notes"] = json::array();
   json tmp_json;
   
   for( const auto& elem:dict)
   {
      json_getter::get_note(tmp_json, deck_name, note_type, elem.first, elem.second, tags);
      request.at("params").at("notes").push_back( tmp_json );
   }
}

void json_getter:: get_add_notes(json& request,const std::string & deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags)
{
   request["action"] = "addNotes";
   request["version"] = 6;
   request["params"]["notes"] = json::array();
   json tmp_json;
   
   for( const auto& elem:dict)
   {
      json_getter::get_note(tmp_json, deck_name, note_type, elem.first, elem.second, tags);
      request.at("params").at("notes").push_back( tmp_json );
   }
}


// The data passed from curl to this function are not null-terminated
size_t curl_write_cb(void* buffer, size_t size, size_t nmemb, void* userp)
{
   static_cast<std::string*>(userp)->append(static_cast<char*>(buffer), size * nmemb);
   return size * nmemb;
}

std::string trim_string(const std::string& str, const std::string& trim)
{
   const auto str_begin{ str.find_first_not_of(trim) };
   if (str_begin == std::string::npos)
      return "";  // str contains only 'trim' chars
   const auto str_end{ str.find_last_not_of(trim) };
   const auto range = str_end - str_begin +1;
   return str.substr(str_begin, range);
}

bool parse_line( const std::string& line, std::string& key, std::string& value)
{
   size_t split_pos = line.find('-');
   if( split_pos == std::string::npos )
   {
      return false;
   }
   key = trim_string(line.substr(0, split_pos));
   value = trim_string(line.substr(split_pos + 1));
   if( key.empty() || value.empty() )
      return false;
   return true;
}

std::string to_lowercase( const std::string& str )
{
   std::string res{str};
   std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c){ return std::tolower(c);});
   return res;
}

void process_file( const char* file_path, std::unordered_map<std::string, std::string>& dict)
{
   std::ifstream file( file_path );
   if(!file)
      my_exception("Error: Can't open input file.");
   
   std::string line;
   int line_num{ 0 };
   std::string key, value;
   std::unordered_set<std::string> duplicity_test;
   while (getline(file, line))
   {
      ++line_num;
      if(line.empty())
         continue;
      
      if( !parse_line(line, key, value) )
      {
         std::cout<<"Unprocessed line #"<<line_num<<" .Contains invalid data. "<<line<<std::endl;
         continue;
      }
      
      if( duplicity_test.count(to_lowercase(key)) )
      {
         std::cout<<"Line "<<line_num<<" contains duplicate data and is ignored: "<<line<<std::endl;
         continue;
      }
      else
      {
         duplicity_test.insert(to_lowercase(key));
         dict[key] = value;
      }
   }
}

CURLcode send_request(CURL* curl, const json& request, std::string& response )
{
   curl_easy_setopt(curl, CURLOPT_URL, AnkiConnect);
   curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, request.dump().c_str());
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
   return curl_easy_perform(curl);
}

void test_ankiconnect_response( const json& response )
{
   bool is_ok{ response.at("error").is_null() };
   if( !is_ok)
      throw my_exception( response.at("error"), my_exception::ANKI);
}

void test_deck_name(CURL* curl, const std::string& deck_name)
{
   CURLcode code;
   std::string response;
   json request;
   json_getter::get_deck_names(request);
   
   code = send_request( curl, request, response );
   if (code != CURLE_OK)
      throw my_exception(curl_easy_strerror(code), my_exception::CURL);
   
   json response_json = json::parse( response );
   test_ankiconnect_response(response_json);
   response_json = response_json.at("result");
   if( std::find(response_json.begin(), response_json.end(), deck_name ) == response_json.end())
      throw my_exception("Error: Deck '"+deck_name+"' not found.");
}

std::string normalize_string(const char* str)
{
   UErrorCode uerrc{ U_ZERO_ERROR};
   const icu_67::Normalizer2* norm{ icu_67::Normalizer2::getNFCInstance(uerrc)};
   if(U_FAILURE(uerrc))
      throw my_exception(u_errorName_67(uerrc), my_exception::ICU);
   
   icu_67::UnicodeString ustr_string(str);
   icu_67::UnicodeString ustr_string_normalized{ norm->normalize(ustr_string, uerrc)};
   if(U_FAILURE(uerrc))
      throw my_exception(u_errorName_67(uerrc), my_exception::ICU);
   
   std::string normalized_string;
   ustr_string_normalized.toUTF8String(normalized_string);
   return normalized_string;
}

void test_note_type(CURL* curl, const std::string& note_type)
{
   CURLcode code;
   std::string response;
   json request;
   json_getter::get_note_types(request);
   
   code = send_request( curl, request, response );
   if (code != CURLE_OK)
      throw my_exception(curl_easy_strerror(code), my_exception::CURL);
   
   json response_json = json::parse( response );
   test_ankiconnect_response(response_json);  // modify to return void and to throw exceptions
   response_json = response_json.at("result");
   if( std::find(response_json.begin(), response_json.end(), note_type ) == response_json.end() )
      throw my_exception("Error: Note type '" + note_type + "' not found.");
}

void test_can_add_notes(CURL* curl, const std::string& deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags)
{
   
   CURLcode code;
   std::string response_str;
   json request;
   json_getter::get_can_add_notes(request, deck_name, note_type, dict, tags);
   
   code = send_request( curl, request, response_str );
   if (code != CURLE_OK)
      throw my_exception(curl_easy_strerror(code),my_exception::CURL);

   json response_json = json::parse( response_str );
   test_ankiconnect_response(response_json);  // modify to return void and to throw exceptions
   std::vector<bool> result = response_json.at("result").get<std::vector<bool>>();
   
   auto v_iter = result.begin();
   for( const auto& elem : dict)
   {
      if( !(*v_iter))
         std::cout<<"Can't add note: "<<elem.first<<" - "<<elem.second<<std::endl;
      ++v_iter;
   }
   
   if( !std::all_of(result.begin(), result.end(), [](bool b){return b;}) )
      throw my_exception("Error: Can't add all notes.");
}

void add_notes(CURL* curl, const std::string& deck_name, const std::string& note_type, std::unordered_map<std::string,std::string>& dict, const std::vector<std::string>& tags)
{
   
   CURLcode code;
   std::string response_str;
   json request;
   json_getter::get_add_notes(request, deck_name, note_type, dict, tags);
   
   code = send_request( curl, request, response_str );
   if (code != CURLE_OK)
      throw my_exception(curl_easy_strerror(code), my_exception::CURL);

   json response_json = json::parse( response_str );
   test_ankiconnect_response(response_json);  // modify to return void and to throw exceptions

   auto dict_iter{ dict.begin()};
   int count{ 0 };
   for(const auto& elem:response_json.at("result"))
   {
      if(elem.is_null())
         std::cout<<"Note not added: "<<dict_iter->first<<" - "<<dict_iter->second<<std::endl;
      else
         ++count;
      ++dict_iter;
   }

   if( !std::all_of(response_json.at("result").begin(), response_json.at("result").end(), [](json o){return !o.is_null();}))
      my_exception("Error: Not all cards have been created.");
   std::cout<<count<<" notes( "<<count*2<<" cards) have been created."<<std::endl;
}
