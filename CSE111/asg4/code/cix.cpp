// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"
using wordvec = vector<string>;

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"get", cix_command::GET  },
   {"put", cix_command::PUT  },
   {"rm"  , cix_command::RM  },
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      size_t host_nbytes = ntohl (header.nbytes);
      auto buffer = make_unique<char[]> (host_nbytes + 1);
      recv_packet (server, buffer.get(), host_nbytes);
      outlog << "received " << host_nbytes << " bytes" << endl;
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}

// The command GET uses a FILE
void cix_get (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::GET;
   strcpy (header.filename, filename.c_str());
   outlog << "sending header " << header << endl;
   send_packet ( server, &header, sizeof header);
   recv_packet ( server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if( header.command != cix_command::FILEOUT) {
      outlog << filename << " does not exist on the server" << endl;
      outlog << "server returned " << header << endl;
   } else {
      size_t host_nbytes = ntohl (header.nbytes);
      if ( host_nbytes > 0 ){
         auto buffer = make_unique<char[]> (host_nbytes + 1);
         recv_packet (server, buffer.get(), host_nbytes);
         outlog << "received " << host_nbytes << " bytes" << endl;
         ofstream output ( filename, ofstream::out);
         output.write(buffer.get(), host_nbytes);
         output.close();
      }
   }
}

// The command PUT uses a FILE.
void cix_put (client_socket& server, string filename ){
   cix_header header;
   strcpy (header.filename, filename.c_str());
   ifstream file (filename, ifstream::binary);
   // Check if the file is open.
   if( file.is_open() ){
      // Get the file length
      file.seekg(0, file.end);
      int file_length = file.tellg();
      outlog << "file_length = " << file_length << endl;
      // Save the host information
      size_t host_nbytes = ntohl (header.nbytes);
      header.command = cix_command::PUT;
      header.nbytes = htonl(file_length);
      outlog << "sending header " << header << endl;
      //outlog << "header nybtes = " << header.nbytes << endl;
      send_packet (server, &header, sizeof header);
      int header_nbytes = ntohl(header.nbytes);
      
      
      if ( file_length != 0 ){
         auto buffer = make_unique<char[]> (header_nbytes + 1);
         file.read(buffer.get(), file_length);
         send_packet (server, buffer.get(), file_length);
      }
      recv_packet (server, &header, sizeof header);
   } else {
      outlog << filename << " does not exist clientside" << endl;
   }
   // ACK NAK Check.
   if (header.command == cix_command::ACK){
      // ACK = success
      outlog << "ACK was received, put "
      << filename << " on the server "
      << endl;
   }
   if (header.command == cix_command::NAK){
      // NAK = failure.
      outlog << "NAK was received, put for " << filename << " failed"
       << endl;
   }
   // Close the file.
   file.close();
}

void cix_rm (client_socket& server, string filename){
   cix_header header;
   header.command = cix_command::RM;
   strcpy (header.filename, filename.c_str());
   // Set the nbytes of header to 0.
   header.nbytes = 0;
   // Send and receive header to do the RM command.
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   // ACK NAK Check.
   if( header.command == cix_command::ACK ){
      // ACK is success, so the file was deleted.
      outlog << "ACK was received, " << filename << " was deleted"
      << endl;
   }
   if( header.command == cix_command::NAK ){
      // NAK is a failure, so the file was not deleted.
      outlog << "NAK was received, " << filename << " was NOT deleted"
      << endl;
   }
}

void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

wordvec split (const string& line, const string& delimiters) {
   wordvec words;
   size_t end = 0;

   // Loop over the string, splitting out words, and for each word
   // thus found, append it to the output wordvec.
   for (;;) {
      size_t start = line.find_first_not_of (delimiters, end);
      if (start == string::npos) break;
      end = line.find_first_of (delimiters, start);
      words.push_back (line.substr (start, end - start));
   }
   return words;
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   // Decalre the host and port variables.
   string host;
   in_port_t port;
   // Go through the different args conditions.
   if (args.size() > 2) usage();
   if (args.size() == 1){ // if args size is 1, host is local.
      host = "localhost";
      port = get_cix_server_port (args, 0);
      outlog << to_string (hostinfo()) << endl;
   } else { // else, the host is not local, and is an argument.
      host = get_cix_server_host (args, 0);
      port = get_cix_server_port (args, 1);
      outlog << to_string (hostinfo()) << endl;
   }

   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         outlog << "command " << line << endl;
         wordvec words = split (line, " \t");
         if ( words.size() == 0 ){
            continue;
         }
         // else, do the following.
         const auto& itor = command_map.find (words[0]);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::GET:
               // for the filename, use the
               // 2nd argument in input_vector.
               if ( words.size() > 1 ){
                  cix_get (server, words[1]);
               }
               else {
                  // print error
               }
               break;
            case cix_command::PUT:
               // for the filename, use the
               // 2nd argument in input_vector.
               if ( words.size() > 1 ){
                  cix_put (server, words[1]);
               }
               break;
            case cix_command::RM:
               // for the filename, use the
               // 2nd argument in input_vector.
               if ( words.size() > 1 ){
                  cix_rm (server, words[1]);
               }
               break;
            default:
               outlog << line << ": invalid command" << endl;
               break;
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
