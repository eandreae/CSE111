// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
   // Initialize the root directory.
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   // Set CWD to root
   cwd  = root;
   // Populate the dirents with .
   root->contents->addEntry(".", root);
   // Populate the dirents with ..
   root->contents->addEntry("..", root);
}

inode_state::~inode_state() {
   // this calls another function, which calls another function, etc.
   root->recurseDestroy();
}

inode_ptr inode_state::get_cwd() {
   return cwd;
}

inode_ptr inode_state::get_root() {
   return root;
}

void inode_state::set_cwd(inode_ptr new_cwd){
   cwd = new_cwd;
}

void inode_state::print_path(wordvec path_names) {
   
   for(int iter = path_names.size()-1; iter != -1; iter--){
      // Pop path_names, storing and printing the string.
      string result = path_names[iter];
      cout << result;
      if ( iter != 0 && result != "/"){
         cout << "/";
      }
   }
   // Once finished printing the path, add the semicolon.
   cout << ":" << endl;
}

void inode_state::print_path_pwd(wordvec path_names) {
   
   for(int iter = path_names.size()-1; iter != -1; iter--){
      // Pop path_names, storing and printing the string.
      string result = path_names[iter];
      cout << result;
      if ( iter != 0 && result != "/"){
         cout << "/";
      }
   }
   // Once done, print newline.
   cout << endl;
}

wordvec inode_state::get_path() {
   // Get the working directory.
   inode_ptr working_directory = this->get_cwd();
   // Print the path
   // Iterate through the parents of the cwd, pushing back
   // the names of the directories onto a wordvec.
   wordvec path_names;
   while ( working_directory != this->get_root() ){
      // Get the contents of the working directory.
      base_file_ptr wd_contents = working_directory->get_contents();
      // Get the parent of the working directory inode.
      inode_ptr parent = wd_contents->get_parent_inode();
      // Get the contents of the parent inode
      base_file_ptr p_contents = parent->get_contents();
      // Get the dirents of the parent's contents.
      map<string,inode_ptr> p_dirents = p_contents->get_dirents();
      // Iterate through the parent's dirents, looking for
      // when one of the parent's dirents is equal to
      // the current working directory.
      for(auto iter = p_dirents.begin(); iter != p_dirents.end();
         iter++){
         // Store the needed temp values
         string file_name = iter->first;
         inode_ptr ptr = iter->second;
         // Check if ptr is the cwd.
         if ( ptr == working_directory ){
            // push the file_name string back onto the wordvec.
            path_names.push_back(file_name);
         }
         // Keep iterating.
      }
      // Outside of the for loop, set the working_directory to the
      // parent inode.
      working_directory = parent;
   }

   if ( working_directory == this->get_root() ){
      // Push back "/" onto the wordvec.
      path_names.push_back("/");
   }
   
   // return the path_names wordvec
   return path_names;
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

void inode::recurseDestroy() {
   contents->recurseDestroy();
   contents = nullptr;
}

base_file_ptr inode::get_contents() {
   return contents;
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::addEntry (const string&, inode_ptr) {
   throw file_error ("is a " + error_file_type());
}

void base_file::recurseDestroy() {
   // Nothing needs to be done.
}

void base_file::printDirents() {
   // Nothing needs to be done.
}

map<string,inode_ptr> base_file::get_dirents() {
   // Nothing needs to be done.
   map<string,inode_ptr> dummy;
   return dummy;
}

inode_ptr base_file::check_dirents(string&) {
   // Nothing needs to be done.
   return nullptr;
}

void directory::recurseDestroy() {
   
   for ( auto iter = dirents.begin(); iter != dirents.end(); iter++ ){
      string file_name = iter->first;
      inode_ptr ptr = iter->second;

      if ( file_name == (".") || file_name == ("..") ){
         // Do nothing
      }
      else {
         // Perform recursion
         ptr->recurseDestroy();
      }
   }
   dirents.clear();
}


size_t plain_file::size() const {
   string temp;
   int total_size = 0;

   for ( unsigned int iter = 0; iter < data.size(); iter++ ){
      temp = data[iter];
      total_size += temp.length();
   }

   total_size += data.size()-1;

   return total_size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   // Iterate through the wordvec, ignoring positions 0 and 1.
   if ( words.size() < 3 ){
      // If there were no words given.
      // Do nothing for now
   }
   for(unsigned int iter = 2; iter != words.size(); iter++ ){
      // push the words[iter] back onto the data wordvec.
      data.push_back(words[iter]);
   }
}

bool plain_file::isDirectory() {
   return is_directory;
}

inode_ptr plain_file::get_parent_inode() {
   inode_ptr dummy;
   return dummy;
}

size_t directory::size() const {
   int size = 0;
   size = dirents.size();
   return size;
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   // dirents[key] = value;
   inode_ptr new_d = make_shared<inode>(file_type::DIRECTORY_TYPE);
   // Get the contents of new_d.
   base_file_ptr new_d_contents = new_d->get_contents();
   // Add the dot and dotdot
   new_d_contents->addEntry(".", new_d);
   inode_ptr dotdot = dirents["."];
   new_d_contents->addEntry("..", dotdot);
   // Add the new directory to this.
   addEntry(dirname, new_d);
   // Return the new_d
   return new_d;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   // Make a plain_file inode.
   inode_ptr new_f = make_shared<inode>(file_type::PLAIN_TYPE);
   // Get the current directory.
   inode_ptr curr_d = dirents["."];
   // Add the new file to the directory.
   curr_d->get_contents()->addEntry(filename, new_f);
   // return the new file ptr.
   return new_f;
}

void directory::addEntry (const string& keyName, inode_ptr value) {
   // Insert the key/value pair into the dirents map.
   dirents[keyName] = value;
}

map<string,inode_ptr> directory::get_dirents() {
   return dirents;
}

bool directory::isDirectory() {
   return is_directory;
}

void directory::printDirents() {

   for ( auto iter = dirents.begin(); iter != dirents.end(); iter++ ){
      string file_name = iter->first;
      inode_ptr ptr = iter->second;

      if ( file_name == "." || file_name == ".." ){
         cout << "   " << ptr->get_inode_nr() << "        "
         << ptr->get_contents()->size() << "  " << file_name
         << "/" << endl;
      }
      else {
         // Check if it is a directory
         if ( ptr->get_contents()->isDirectory() ){
            // If it is, print the name with the
            // "/" at the end of the name.
            cout << "   " << ptr->get_inode_nr() << "        "
            << ptr->get_contents()->size() << "  " << file_name
            << "/" << endl;
         }
         else {
            // Otherwise, print the file name normally.
            cout << "   " << ptr->get_inode_nr() << "        "
            << ptr->get_contents()->size() << "  " << file_name
            << endl;
         }
      }
   }
}

inode_ptr directory::check_dirents(string& filename){
   // iterate through this directory to see if filename is
   // a member of its dirents.
   string target = filename;

   for(auto iter = dirents.begin(); iter != dirents.end(); iter++){
      string file_name = iter->first;
      inode_ptr ptr = iter->second;

      if( file_name == target ){
         return iter->second;
      }
      else {
         // Keep iterating.
      }
   }
   // Return nullptr
   return nullptr;
}

inode_ptr directory::get_parent_inode() {
   // Save the inode_ptr of the parent of this directory.
   inode_ptr parent = dirents[".."];
   return parent;
}
