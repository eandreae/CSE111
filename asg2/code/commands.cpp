// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << "sugma dicc gottem" << endl;
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ( words.size() < 2 ){
      state.set_cwd(state.get_root());
   }
   else {
      // Save the target word.
      string target = words[1];
      // Split the target word into a wordvec.
      wordvec split_result = split(target, "/");
      // Checker bool, for later.
      bool target_found = false;
      // Get the current working directory, store it in a
      // temporary directory for iteration.
      inode_ptr temp_directory = state.get_cwd();
      // Iterate through split_result, until the end of it.
      // Checking if the directories exist on the way.
      for(unsigned int iter = 0; iter < split_result.size(); iter++){
         // Get the contents of the temp_directory.
         base_file_ptr temp_contents = temp_directory->get_contents();
         // Check if the dirents have split_result[0-size()-1].
         inode_ptr dirent_check = temp_contents->check_dirents(
            split_result[iter]);
         // Check if dirent_check is null.
         if ( dirent_check != nullptr ){
            // If it is not null, set the new temp_directory to it.
            temp_directory = dirent_check;
            // Check if iter = split_result.size()-1
            if ( iter == (split_result.size()-1) ){
               // It has reached the end of the wordvec,
               // Therefore the target has been found.
               target_found = true;
            }
         }
      }
      // Outside of the for loop, check if the target was found.
      if ( target_found ) {
         // Set the cwd to temp_directory.
         state.set_cwd(temp_directory);
      }
      else {
         // Print error message.
         cout << target << ": Does not exist" << endl;
      }
   }  
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   wordvec path_names;

   inode_ptr working_directory = state.get_cwd();

   if ( words.size() < 2 ){
      // for when ls "" is called.
      path_names = state.get_path();
      // Print the path of path_names.
      state.print_path(path_names);
      // Print the dirents of the working directory.
      working_directory->get_contents()->printDirents();
   }
   else {
      // Remember the base cwd
      inode_ptr original_cwd = state.get_cwd();
      // Set up an error string path
      // Iterate through the wordvec
      for(unsigned int w_iter = 1; w_iter < words.size(); w_iter++){
         // Save the target word.
         string target = words[w_iter];
         // Split the target word into a wordvec.
         wordvec split_result;
         if ( target != "/" ){
            split_result = split(target, "/");
         }
         else {
            split_result.push_back("/");
         }
         // Checker bool, for later.
         bool target_found = false;
         // Get the current working directory, store it in a
         // temporary directory for iteration.
         inode_ptr temp_directory = state.get_cwd();
         // Check if the split_result is size 1.
         if ( split_result.size() == 1 ){
            // if it is size 1, there are many different cases.
            // "/", ".", "..", "filename"

            // Case 1: "/"
            if ( split_result[0] == "/" ){
               // Do an ls of the root.
               // set the cwd to root.
               state.set_cwd(state.get_root());
               // Get the path
               path_names = state.get_path();
               // Print the path of root.
               state.print_path(path_names);
               // Print the dirents of root.
               state.get_root()->get_contents()->printDirents();
               // Set the cwd back to the original cwd.
               state.set_cwd(original_cwd);
               // Set target_found to true;
               target_found = true;
            }
            // Case 2: "."
            else if ( split_result[0] == "." ){
               // Do an ls of itself.
               // Don't change the cwd.
               // Set the path to "."
               path_names.push_back(".");
               // Print the path
               state.print_path(path_names);
               // Print the dirents of the cwd.
               state.get_cwd()->get_contents()->printDirents();
               // Set target_found to true;
               target_found = true;
            }
            // Case 3: ".."
            else if ( split_result[0] == ".." ){
               // Do an ls of its parent
               // Get the parent inode of the cwd.
               inode_ptr parent = state.get_cwd()->get_contents()
               ->get_parent_inode();
               // Change the cwd to the parent of the cwd.
               state.set_cwd(parent);
               // Set the path to ".."
               path_names.push_back("..");
               // Print the path
               state.print_path(path_names);
               // Print the dirents of the cwd.
               state.get_cwd()->get_contents()->printDirents();
               // Set the cwd back to the original cwd.
               state.set_cwd(original_cwd);
               // Set target_found to true;
               target_found = true;
            }
            else {
               // Otherwise, it is a filename.
               // Search for it in the root directory.
               // Set the cwd to root
               state.set_cwd(state.get_root());
               // Check the cwd for split_result[0].
               inode_ptr exists = state.get_cwd()->get_contents()
               ->check_dirents(split_result[0]);
               // check if "exists" is nullptr.
               if ( exists == nullptr ){
                  // print error message.
               }
               else {
                  // The directory exists.
                  // Set the cwd to exists.
                  state.set_cwd(exists);
                  // Get the path of cwd.
                  path_names = state.get_path();
                  // Print the path
                  state.print_path(path_names);
                  // Print the dirents of the cwd.
                  state.get_cwd()->get_contents()->printDirents();
                  // Set the cwd back to the original cwd.
                  state.set_cwd(original_cwd);
                  // Set target_found to true.
                  target_found = true;
               }
            }
         }
         else {
            // Iterate through split_result, until the end of it.
            // Checking if the directories exist on the way.
            for(unsigned int iter = 0; iter < split_result.size(); 
               iter++){
               // Get the contents of the temp_directory.
               base_file_ptr temp_contents = temp_directory
                  ->get_contents();
               // Check if the dirents have split_result[0-size()-1].
               inode_ptr dirent_check = temp_contents->check_dirents(
                  split_result[iter]);
               // Check if dirent_check is null.
               if ( dirent_check != nullptr ){
                  // If it is not null, set the new 
                  // temp_directory to it.
                  temp_directory = dirent_check;
                  // Check if iter = split_result.size()-1
                  if ( iter == (split_result.size()-1) ){
                     // It has reached the end of the wordvec,
                     // Therefore the target has been found.
                     target_found = true;
                  }
               }
            }
            if ( target_found ){
               // Set the cwd to the temp_directory
               state.set_cwd(temp_directory);
               // Get the path of the temp_directory.
               path_names = state.get_path();
               // Print the path
               state.print_path(path_names);
               // Print the dirents of the cwd.
               state.get_cwd()->get_contents()->printDirents();
               // Set the cwd back to the original cwd.
               state.set_cwd(original_cwd);
            }
         }
         // Outside of the for loop, check if the target was found.
         if ( target_found ) {
            // Do nothing
         }
         else {
            // Break out of the for loop
            cout << words[w_iter] << ": Does not exist" << endl;
         }
      }
      // Otherwise do nothing.
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ( words.size() < 2 ){
      cout << "mkdir: missing operand" << endl;
   }
   else {
      // Remember the base cwd
      inode_ptr original_cwd = state.get_cwd();
      // Set up an error string path
      // Iterate through the wordvec
      for(unsigned int w_iter = 1; w_iter < words.size(); w_iter++){
         // Save the target word.
         string target = words[w_iter];
         // Split the target word into a wordvec.
         wordvec split_result = split(target, "/");
         // Checker bool, for later.
         bool target_found = false;
         // Get the current working directory, store it in a
         // temporary directory for iteration.
         inode_ptr temp_directory = state.get_cwd();
         // Check if the split_result is size 1.
         if ( split_result.size() == 1 ){
            // If it is just one thing, add it.
            string file_name = split_result[0];
            temp_directory->get_contents()->mkdir(file_name);
            target_found = true;
         }
         else {
            // Iterate through split_result, until the end of it.
            // Checking if the directories exist on the way.
            for(unsigned int iter = 0; iter < split_result.size()-1; 
               iter++){
               // Get the contents of the temp_directory.
               base_file_ptr temp_contents = temp_directory
                  ->get_contents();
               // Check if the dirents have split_result[0-size()-1].
               inode_ptr dirent_check = temp_contents->check_dirents(
                  split_result[iter]);
               // Check if dirent_check is null.
               if ( dirent_check != nullptr ){
                  // If it is not null, set the new 
                  // temp_directory to it.
                  temp_directory = dirent_check;
                  // Check if iter = split_result.size()-1
                  if ( iter == (split_result.size()-2) ){
                     // It has reached the end of the wordvec,
                     // Therefore the target has been found.
                     target_found = true;
                  }
               }
            }
         }
         // Outside of the for loop, check if the target was found.
         if ( target_found ) {
            // Set the cwd to the temp_directory
            state.set_cwd(temp_directory);
            // Get the string dirname 
            string file_name = split_result[split_result.size()-1];
            // add the file_name as a directory.
            state.get_cwd()->get_contents()->mkdir(file_name);
            // Set the cwd back to the original cwd.
            state.set_cwd(original_cwd);
         }
         else {
            // Break out of the for loop
            cout << words[w_iter] << ": Does not exist" << endl;
         }
      }
      // Otherwise do nothing.
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

