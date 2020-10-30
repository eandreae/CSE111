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
   if ( words.size() < 2 ){
      // If just "cat" is put in
      // print an error.
      cout << "cat: missing operand." << endl;
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
         // Checker bool to see if the target exists.
         bool target_found = false;
         // Get the current working directory, store it in a
         // temporary directory for iteration.
         inode_ptr temp_directory = state.get_cwd();
         // Check if the split_result is size 1.
         if ( split_result.size() == 1 ){
            // If the split_result size is 1, search within cwd.
            // Check the dirents of the cwd if split_result[0] exists.
            inode_ptr exists = temp_directory->get_contents()
            ->check_dirents(split_result[0]);
            // If exists is nullptr, it doesn't exist.
            if ( exists == nullptr ){
               // the file doesn't exist.
               // Print an error.
            }
            else if ( exists->get_contents()->isDirectory() ){
               // Otherwise, if exists is a directory,
               // set target to found.
               target_found = true;
            }
            else {
               // Otherwise, the target exists
               // Set the target_found to true.
               target_found = true;
            }
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
               if ( dirent_check == nullptr ){
                  // The file does not exist.
                  // Print error
               }
               else if (dirent_check->get_contents()->isDirectory()){
                  // The file exists, and it is a directory.
                  // Keep going.
                  temp_directory = dirent_check;
                  // Check if we've reached the last directory.
                  if ( iter == split_result.size()-2 ){
                     // If we've made it here, we're at the target.
                     target_found = true;
                  }
               }
            }
         }
         // Outside of the for loop, check if the target was found.
         if ( target_found ) {
            // Set the cwd to the temp_d.
            state.set_cwd(temp_directory);
            // Check if the file_name exists.
            inode_ptr exists = state.get_cwd()->get_contents()
            ->check_dirents(split_result[split_result.size()-1]);
            // Check if exists is nullptr.
            if ( exists == nullptr ){
               // If exists is nullptr, then the file does not exist.
               // Print error.
            }
            else if ( exists->get_contents()->isDirectory() ){
               // exists is a directory, print error.
            }
            else {
               // If we get here, it exists and is a file, print it.
               wordvec output = exists->get_contents()->readfile();
               cout << output << endl;
            }
            // Set the cwd back to the original cwd.
            state.set_cwd(original_cwd);
         }
         else {
            // Break out of the for loop
            cout << words[w_iter] << ": Does not exist" << endl;
         }
      }
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ( words.size() < 2 || words[1] == "/" ){
      state.set_cwd(state.get_root());
   }
   else {
      // Save the target word.
      string target = words[1];
      // Split the target word into a wordvec.
      wordvec split_result = split(target, "/");
      // Checker bool for if the target exists.
      bool target_found = false;
      // Checker bool for if the target is a directory.
      bool is_directory = false;
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
            // Save if it is a directory.
            is_directory = dirent_check->get_contents()
            ->isDirectory();
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
         // Check if it is a directory.
         if ( is_directory ){
            // If the target is a directory,
            // Set the cwd to temp_directory.
            state.set_cwd(temp_directory);
         }
         else {
            // It is not a directory, do not change the cwd.
            // Instead, print an error.
            cout << target << " is not a directory." << endl;
         }
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
               // Search for it in the current directory.
               // Check the cwd for split_result[0].
               inode_ptr exists = state.get_cwd()->get_contents()
               ->check_dirents(split_result[0]);
               // check if "exists" is nullptr.
               if ( exists == nullptr ){
                  // print error message.
               }
               else {
                  // Check if exists is a directory.
                  if ( exists->get_contents()->isDirectory() ){
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
                  // Check if dirent_check is a directory.
                  if ( dirent_check->get_contents()->isDirectory() ){
                     // Set temp_directory to it.
                     temp_directory = dirent_check;
                     // Check if iter = split_result.size()-1
                     if ( iter == (split_result.size()-1) ){
                        // It has reached the end of the wordvec,
                        // Therefore the target has been found.
                        target_found = true;
                     }
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
   
   // Remember the base cwd
   inode_ptr original_cwd = state.get_cwd();
   // Store an empty wordvec.
   wordvec empty;
   
   if ( words.size() < 2 ){
      // if "lsr" is called by itself
      // Run lsr on the current state.
      lsr_recursion_root(state, words);
      // Set the cwd to the original cwd.
      state.set_cwd(original_cwd);
   }
   // Otherwise, continue the program.

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
            // Do an lsr of the root.
            // set the cwd to root.
            state.set_cwd(state.get_root());
            // Perform the recursion
            lsr_recursion_root(state, empty);
            // Return the cwd to the original wd.
            state.set_cwd(original_cwd);
            // Set target_found to true.
            target_found = true;
         }
         // Case 2: "."
         else if ( split_result[0] == "." ){
            // Do an lsr of itself.
            // Perform recursion on itself.
            lsr_recursion_root(state, empty);
            // Set the cwd back to the original cwd.
            state.set_cwd(original_cwd);
            // Set target_found to true.
            target_found = true;
         }
         // Case 3: ".."
         else if ( split_result[0] == ".." ){
            // Do an lsr of its parent
            // Get the parent inode of the cwd.
            inode_ptr parent = state.get_cwd()->get_contents()
            ->get_parent_inode();
            // Change the cwd to the parent of the cwd.
            state.set_cwd(parent);
            // Perform recursion
            lsr_recursion_root(state, empty);
            // Set the cwd back to the original cwd.
            state.set_cwd(original_cwd);
            // Set target_found to true.
            target_found = true;
         }
         else {
            // Otherwise, it is a filename.
            // Search for it in the current directory.
            // Check the cwd for split_result[0].
            inode_ptr exists = state.get_cwd()->get_contents()
            ->check_dirents(split_result[0]);
            // check if "exists" is nullptr.
            if ( exists == nullptr ){
               // print error message.
            }
            else {
               // Check if exists is a directory.
               if ( exists->get_contents()->isDirectory() ){
                  // Set the cwd to exists.
                  state.set_cwd(exists);
                  // Perform recursion
                  lsr_recursion_root(state, empty);
                  // Reset the cwd.
                  state.set_cwd(original_cwd);
                  // Set target_found to true.
                  target_found = true;
               }
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
               // Check if dirent_check is a directory.
               if ( dirent_check->get_contents()->isDirectory() ){
                  // Set temp_directory to it.
                  temp_directory = dirent_check;
                  // Check if iter = split_result.size()-1
                  if ( iter == (split_result.size()-1) ){
                     // It has reached the end of the wordvec,
                     // Therefore the target has been found.
                     target_found = true;
                  }
               }
            }
         }
         if ( target_found ){
            // Set the cwd to the temp_directory
            state.set_cwd(temp_directory);
            // Perform recusrion.
            lsr_recursion_root(state, empty);
            // Reset cwd.
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
}

void lsr_recursion_root (inode_state& state, const wordvec& words) {
   // Print current directory.
   fn_ls(state, words);

   //Scan through the dirents for a directory.
   map<string,inode_ptr> dirents = state.get_cwd()->get_contents()
   ->get_dirents();

   for(auto iter = dirents.begin(); iter != dirents.end(); iter++){
      string file_name = iter->first;
      inode_ptr ptr = iter->second;
      // Check if the ptr is a directory.
      if( file_name == "." || file_name == ".." ){
         // Do nothing.
      }
      else {
         // Check if the ptr is a directory.
         if ( ptr->get_contents()->isDirectory() ){
            // Set the cwd to ptr.
            state.set_cwd(ptr);
            // lsr on the new state.
            lsr_recursion_root(state, words);
         }
      }
   }
}

void lsr_recursion (inode_state& state, const wordvec& words) {

   fn_pwd(state, words);

   //Scan through the dirents for a directory.
   map<string,inode_ptr> dirents = state.get_cwd()->get_contents()
   ->get_dirents();

   for(auto iter = dirents.begin(); iter != dirents.end(); iter++){
      string file_name = iter->first;
      inode_ptr ptr = iter->second;
      // Check if the ptr is a directory.
      if( file_name == "." || file_name == ".." ){
         // Do nothing.
      }
      else {
         // Check if the ptr is a directory.
         cout << file_name << endl;
         if ( ptr->get_contents()->isDirectory() ){
            // Set the cwd to ptr.
            state.set_cwd(ptr);
            // lsr on the new state.
            fn_pwd(state, words);
            fn_ls(state, words);
            lsr_recursion(state, words);
         }
      }
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ( words.size() < 2 ){
      // If they said make <path> with nothing following, or less.
      // Print error message.
      cout << "make: missing operand" << endl;
   }
   else {
      // Save the original cwd.
      inode_ptr original_cwd = state.get_cwd();
      // Save the target word.
      string target = words[1];
      // Split the target word into a wordvec.
      wordvec split_result = split(target, "/");
      // Checker bool for if the target exists.
      bool target_found = false;
      // Get the current working directory, store it in a
      // temporary directory for iteration.
      inode_ptr temp_directory = state.get_cwd();
      // Iterate through split_result, until the end of it.
      // Checking if the directories exist on the way.
      if ( split_result.size() == 1 ){
         // Make the file in the current directory.
         // Set target_found to true.
         target_found = true;
      }
      for(unsigned int iter = 0; iter < split_result.size()-1; iter++){
         // Get the contents of the temp_directory.
         base_file_ptr temp_contents = temp_directory->get_contents();
         // Check if the dirents have split_result[0-size()-1].
         inode_ptr dirent_check = temp_contents->check_dirents(
            split_result[iter]);
         // Check if dirent_check is not nullptr
         if ( dirent_check != nullptr ){
            // Check if the dirent_check is a directory.
            if ( dirent_check->get_contents()->isDirectory () ) {
               // if the dirent_check is a directory, keep going.
               temp_directory = dirent_check;
               // Check if we've iterated to the last directory.
               if ( iter == (split_result.size()-2) ){
                  // If we've made it here, the target has been found.
                  target_found = true;
               }
            }
         }
      }
      // Outside of the for loop, check if the target was found.
      if ( target_found ) {
         // Set the cwd to the temp_directory.
         state.set_cwd(temp_directory);
         // Check if the file_name exists.
         inode_ptr exists = state.get_cwd()->get_contents()
         ->check_dirents(split_result[split_result.size()-1]);
         // if exists is nullptr, then it doesn't exist, make it.
         if ( exists == nullptr ){
            // file_name doesn't exist, make it.
            inode_ptr input_file = state.get_cwd()->get_contents()
            ->mkfile(split_result[split_result.size()-1]);
            // Write to the file the rest of words.
            input_file->get_contents()->writefile(words);
         }
         else if ( exists->get_contents()->isDirectory() ){
            // It is not a directory, do not change the cwd.
            // Instead, print an error.
            cout << target << " is a directory." << endl;
         }
         else {
            // If it gets here, then the file_name already exists
            // and is not a directory, so it is a plain_file.
            // Therefore, we re-write the information.
            exists->get_contents()->writefile(words);
         }
         // Set the cwd back to the original cwd.
         state.set_cwd(original_cwd);
      }
      else {
         // Print error message.
         cout << target << ": Does not exist" << endl;
      }
   }
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
            // If the split_result size is 1, just add it on.
            // Set the target_found to true.
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
            // Check if the file_name already exists in the directory.
            inode_ptr exists = temp_directory->get_contents()
            ->check_dirents(file_name);
            // if exists is nullptr, add it like normal.
            if ( exists == nullptr ){
               // add the file_name as a directory.
               state.get_cwd()->get_contents()->mkdir(file_name);
            }
            else {
               // Otherwise, it already exists, print error.
               cout << words[w_iter] << " already exists." << endl;
            }
            
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
   // Get the current working directory and print the path.
   wordvec path_names = state.get_path();
   // Print the path.
   state.print_path_pwd(path_names);
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

