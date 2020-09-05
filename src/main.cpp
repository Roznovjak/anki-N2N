#include <cassert>

#include "helpers.hpp"


// expects following program arguments: file_path deck_name note_type [tag ...]
int main( int argc, char* argv[] )
{
   std::vector<std::string> tags;
   if (argc < num_of_required_program_args)
   {
      std::cerr<< "Error: Wrong number of arguments." << std::endl;
      return 1;
   }
   else if( argc > num_of_required_program_args)
   {
      std::string str_tags;
      for( int i=num_of_required_program_args; i<argc; ++i)
         tags.emplace_back(argv[i]);
   }
   
   std::string deck_name_normalized{ normalize_string(argv[2])};
   std::string note_type_normalized{ normalize_string(argv[3])};
   std::unordered_map<std::string, std::string> dict;
   process_file( argv[1], dict );
   
   curl_global_init(CURL_GLOBAL_DEFAULT);
   CURL* curl{ curl_easy_init() };

   if (curl)
   {
      try{
         test_deck_name(curl, deck_name_normalized );
         test_note_type(curl, note_type_normalized );
         test_can_add_notes(curl, deck_name_normalized, note_type_normalized, dict, tags);
         add_notes(curl, deck_name_normalized, note_type_normalized, dict, tags);
      }
      catch( const std::exception& e)
      {
         std::cerr << e.what() << std::endl;;
      }
      
      curl_easy_cleanup(curl);
   }
   else
   {
      std::cerr << "Error initializing curl." << std::endl;
   }

   curl_global_cleanup();
   return 0;
}
