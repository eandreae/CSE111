// $Id: cixd.cpp,v 1.8 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

bool checkValidFileName (const cix_header& header){
   // Checking .filename
   // remember .filename has a length
   // The purpose of this function is to check
   // if the filename in the header is null terminated
   // within the bounds of the filename
   for( int iter = 0; iter != FILENAME_SIZE; ++iter ){
      // Looking for null character and slash "/"
      char ch = header.filename[iter];
      if ( ch == '\0' ){
         return true;
      }
      if ( ch == '/' ){
         return false;
      }
   }
   return false;
}

void reply_ls (accepted_socket& client_sock, cix_header& header) {
   const char* ls_cmd = "ls -l 2>&1";
   FILE* ls_pipe = popen (ls_cmd, "r");
   if (ls_pipe == NULL) {
      outlog << "ls -l: popen failed: " << strerror (errno) << endl;
      header.command = cix_command::NAK;
      header.nbytes = htonl (errno);
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   string ls_output;
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      ls_output.append (buffer);
   }
   int status = pclose (ls_pipe);
   if (status < 0) outlog << ls_cmd
   << ": " << strerror (errno) << endl;
              else outlog << ls_cmd << ": exit " << (status >> 8)
                          << " signal " << (status & 0x7F)
                          << " core " << (status >> 7 & 1) << endl;
   header.command = cix_command::LSOUT;
   header.nbytes = htonl (ls_output.size());
   memset (header.filename, 0, FILENAME_SIZE);
   outlog << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   outlog << "sent " << ls_output.size() << " bytes" << endl;
}
void reply_get(accepted_socket& client_sock, cix_header& header) {
   // First check if the filename is valid.
   if ( !checkValidFileName(header) ) {
      // In this case, the file does not exist, report a failure.
      // Set the header.command to NAK for failure.
      header.command = cix_command::NAK;
      // Send the information packet to
      // the server to report the failure.
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   // Get the file. Using std::ifstream
   std::ifstream file (header.filename, std::ifstream::binary);
   // Check if the file exists with an if statement.
   if ( file ){
      // Go to the end of the file using seekg.
      file.seekg(0, file.end);
      // Get the length of the file through a tell.
      int file_length = file.tellg();
      // Reset the file to the beginning.
      file.seekg(0, file.beg);
      header.nbytes = htonl (file_length);
      // Set the header command to FILE.
      header.command = cix_command::FILEOUT;
      // Send the information packet to the server, to do the GET.
      send_packet (client_sock, &header, sizeof header);
      
      if ( file_length != 0 ){
         // Send the payload over.
         auto buffer = make_unique<char[]> (file_length);
         file.read(buffer.get(), file_length);
         send_packet (client_sock, buffer.get(), file_length);
      }
      
   } else {
      // In this case, the file does not exist, report a failure.
      // Set the header.command to NAK for failure.
      header.command = cix_command::NAK;
      // Send the information packet to
      // the server to report the failure.
      send_packet (client_sock, &header, sizeof header);
   }
   // Close the file.
   file.close();
}

void reply_put(accepted_socket& client_sock, cix_header& header) {
   // First check if the filename is valid.
   if ( !checkValidFileName(header) ) {
      // In this case, the file does not exist, report a failure.
      // Set the header.command to NAK for failure.
      header.command = cix_command::NAK;
      // Send the information packet to
      // the server to report the failure.
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   // Get the output file using std::ofstream.
   std::ofstream output ( header.filename, std::ofstream::out);
   // Check if the output file exists with an if statement.
   if( output ){
      size_t host_nbytes = ntohl (header.nbytes);
      outlog << "host_nbytes = " << host_nbytes << endl; 
      if ( host_nbytes != 0 ){
         outlog << "1" << endl;
         auto buffer = make_unique<char[]> (host_nbytes + 1);
         outlog << "2" << endl;
         recv_packet (client_sock, buffer.get(), header.nbytes + 1);
         outlog << "3" << endl;
         output.write(buffer.get(), header.nbytes);
         outlog << "4" << endl;
      }
      // if it exists, then it is a success, so send an ACK.
      // Set the command to ACK, indicating success.
      header.command = cix_command::ACK;
      // Send the information packet to the server.
      send_packet(client_sock, &header, sizeof header);
   } else {
      // Otherwise, no permission to edit, and is a failure.
      // Set the command to NAK, indicating a failure.
      header.command = cix_command::NAK;
      // Do not write the buffer to the file.
      // Send the information packet to the server.
      send_packet(client_sock, &header, sizeof header);
   }
}

void reply_rm (accepted_socket& client_sock, cix_header& header) {
   // First check if the filename is valid.
   if ( !checkValidFileName(header) ) {
      // In this case, the file does not exist, report a failure.
      // Set the header.command to NAK for failure.
      header.command = cix_command::NAK;
      // Send the information packet to
      // the server to report the failure.
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   // Unlink file, check for success
   if (unlink(header.filename) == 0){
      header.command = cix_command::ACK;
      send_packet(client_sock, &header, sizeof header);
   // Success not found, return failure
   }else{
      header.command = cix_command::NAK;
      send_packet(client_sock, &header, sizeof header);
   }
}

void run_server (accepted_socket& client_sock) {
   outlog.execname (outlog.execname() + "-server");
   outlog << "connected to " << to_string (client_sock) << endl;
   try {
      for (;;) {
         cix_header header;
         recv_packet (client_sock, &header, sizeof header);
         outlog << "received header " << header << endl;
         switch (header.command) {
            case cix_command::LS:
               reply_ls (client_sock, header);
               break;
            case cix_command::GET:
               reply_get (client_sock, header);
               break;
            case cix_command::PUT:
               reply_put (client_sock, header);
               break;
            case cix_command::RM:
               reply_rm (client_sock, header);
               break;
            default:
               outlog << "invalid client header:" << header << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   throw cix_exit();
}

void fork_cixserver (server_socket& server, accepted_socket& accept) {
   pid_t pid = fork();
   if (pid == 0) { // child
      server.close();
      run_server (accept);
      throw cix_exit();
   }else {
      accept.close();
      if (pid < 0) {
         outlog << "fork failed: " << strerror (errno) << endl;
      }else {
         outlog << "forked cixserver pid " << pid << endl;
      }
   }
}


void reap_zombies() {
   for (;;) {
      int status;
      pid_t child = waitpid (-1, &status, WNOHANG);
      if (child <= 0) break;
      outlog << "child " << child
             << " exit " << (status >> 8)
             << " signal " << (status & 0x7F)
             << " core " << (status >> 7 & 1) << endl;
   }
}

void signal_handler (int signal) {
   outlog << "signal_handler: caught " << strsignal (signal) << endl;
   reap_zombies();
}

void signal_action (int signal, void (*handler) (int)) {
   struct sigaction action;
   action.sa_handler = handler;
   sigfillset (&action.sa_mask);
   action.sa_flags = 0;
   int rc = sigaction (signal, &action, nullptr);
   if (rc < 0) outlog << "sigaction " << strsignal (signal)
                      << " failed: " << strerror (errno) << endl;
}


int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   signal_action (SIGCHLD, signal_handler);
   in_port_t port = get_cix_server_port (args, 0);
   try {
      server_socket listener (port);
      for (;;) {
         outlog << to_string (hostinfo()) << " accepting port "
             << to_string (port) << endl;
         accepted_socket client_sock;
         for (;;) {
            try {
               listener.accept (client_sock);
               break;
            }catch (socket_sys_error& error) {
               switch (error.sys_errno) {
                  case EINTR:
                     outlog << "listener.accept caught "
                         << strerror (EINTR) << endl;
                     break;
                  default:
                     throw;
               }
            }
         }
         outlog << "accepted " << to_string (client_sock) << endl;
         try {
            fork_cixserver (listener, client_sock);
            reap_zombies();
         }catch (socket_error& error) {
            outlog << error.what() << endl;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}
