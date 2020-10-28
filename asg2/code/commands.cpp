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
      cout << "split_result = " << split_result << endl;
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
   // Get the working directory.
   inode_ptr working_directory = state.get_cwd();
   // Print the path
   if ( state.get_root() == state.get_cwd() ){
      // If root == cwd, then print "/:"
      cout << "/:" << endl;
   }
   else {
      // Print the path
      // Not implemented.
      state.print_path();
   }
   // Print the dirents of the working directory.
   working_directory->get_contents()->printDirents();
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
   inode_ptr t_cwd = state.get_cwd();
   t_cwd->get_contents()->mkdir(words[1]);
   
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

