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

void inode_state::print_path() {
   string output = "/";
   // Get the root's contents
   base_file_ptr root_contents = root->get_contents();
   // Get the root's dirents
   map<string,inode_ptr> r_dirents = root_contents->get_dirents();
   // Iterate over the root_dirents in search of the cwd.
   for (auto iter = r_dirents.begin(); iter != r_dirents.end();
   iter++){
      string file_name = iter->first;
      inode_ptr ptr = iter->second;
      // Check if the root's dirent's inode_ptr = the cwd.
      if ( ptr == cwd ){
         // If so, this is the path, print it.
         output += file_name;
      }
      else {
         // Do nothing.
      }
   }
   output += ":";
   cout << output << endl;
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
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
}

bool plain_file::isDirectory() {
   return is_directory;
}

size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
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
   cout << "oo yeah" << filename << endl;
   return nullptr;
   // will put make_shared<inode>(...)
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
         // Do nothing for now.
         cout << "   " << file_name << endl;
      }
      else {
         // Check if it is a directory
         if ( is_directory ){
            // If it is, print the name with the
            // "/" at the end of the name.
            cout << "   " << file_name << "/" << endl;
         }
         else {
            // Otherwise, print the file name normally.
            cout << "   " << file_name << endl;
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
