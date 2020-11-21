// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>
#include <cassert>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"
#include <libgen.h>

const string cin_name = "-";

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

void keyValue_Function ( istream& infile, const string& filename ) {
    regex comment_regex {R"(^\s*(#.*)?$)"};
    regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
    regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
    str_str_map key_value_map;
    str_str_pair key_value_pair;
    str_str_pair target;
    int counter = 1;
    for (;;) {
       string line;
       getline (infile, line);
       if (infile.eof()) break;
       if( filename == cin_name ){
           cout << "-: " << counter << ": " << line << endl;
       } else {
           cout << filename << ": " << counter << ": " << line << endl;
       }

       smatch result;
       if (regex_search (line, result, comment_regex)) {
          // This condition takes in a comment, and gives
          // it back to the user.
          cout << line << endl;
          counter = counter + 1;
          continue;

       }
       if (regex_search (line, result, key_value_regex)) {
          // This condition takes the following commands.
          // foo = bar > outputs > foo = bar

          // key, value.
          // key and value exist, however,
          // result[2] is empty.
          // run the erase.
          if( result[1] == "" && result[2] == "" ){
              for( auto it = key_value_map.begin(); it
              != key_value_map.end(); ++it){
                  cout << it->first << " = " << it->second
                   << endl;
              }
          } else if( result[2] != "" && result[1] != "" ){
              str_str_pair key_value_pair(result[1], result[2]);
              key_value_map.insert(key_value_pair);
              // = bar     > outputs > foo = bar
              cout << result[1] << " = " << result[2] << endl;
          } else if ( result[1] == "" && result[2] != "" ) {
              for( auto it = key_value_map.begin(); it
              != key_value_map.end(); ++it){
                  if( it->second == result[2] ){
                      cout << it->first << " = " << it->second
                       << endl;
                  }
              }
          } else {
              key_value_map.erase(key_value_map.find(result[1]));
          }

          counter = counter + 1;


       }else if (regex_search (line, result, trimmed_regex)) {
          // This condition takes the following commands.
          // foo > outputs > foo = bar

          // ( given that foo = bar has already been said ).
          //cout << "we got sdfasdfadsf" << endl;
          if( key_value_map.find(result[1]) == key_value_map.end() ){
              cout << result[1] << ": key not found" << endl;
          } else {
              str_str_pair target = *(key_value_map.find(result[1]));
              cout << result[1] << " = " << target.second << endl;
          }

          // Insert the logic here for finding the key.
          // The output would be as follows.
          // cout << result[1] << " = " << result[2] << endl;
          // result[2] here would be the Node that we find.
          counter = counter + 1;
      } else {
          assert (false and "This can not happen.");
       }
    }
}

int main (int argc, char** argv) {
    int status = 0;
    string progname (basename (argv[0]));
    vector<string> filenames (&argv[1], &argv[argc]);
    if (filenames.size() == 0) filenames.push_back (cin_name);
    for (const auto& filename: filenames) {
       if (filename == cin_name) keyValue_Function (cin, filename);
       else {
          ifstream infile (filename);
          if (infile.fail()) {
             status = 1;
             cerr << progname << ": " << filename << ": "
                  << strerror (errno) << endl;
          }else {
             keyValue_Function (infile, filename);
             infile.close();
          }
       }
    }
    return status;
}
