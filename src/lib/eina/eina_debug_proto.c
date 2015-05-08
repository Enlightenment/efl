#include "eina_debug.h"

#ifdef EINA_HAVE_DEBUG

int
_eina_debug_monitor_service_send(int fd, const char op[4],
                                 unsigned char *data, int size)
{
   // send protocol packet. all protocol is an int for size of packet then
   // included in that size (so a minimum size of 4) is a 4 byte opcode
   // (all opcodes are 4 bytes as a string of 4 chars), then the real
   // message payload as a data blob after that
   unsigned char *buf = alloca(4 + 4 + size);
   int newsize = size + 4;
   memcpy(buf, &newsize, 4);
   memcpy(buf + 4, op, 4);
   if (size > 0) memcpy(buf + 8, data, size);
   return write(fd, buf, newsize + 4);
}

void
_eina_debug_monitor_service_greet(void)
{
   // say hello to our debug daemon - tell them our PID and protocol
   // version we speak
   unsigned char buf[8];
   int version = 1; // version of protocol we speak
   int pid = getpid();
   memcpy(buf +  0, &version, 4);
   memcpy(buf +  4, &pid, 4);
   _eina_debug_monitor_service_send(_eina_debug_monitor_service_fd,
                                    "HELO", buf, sizeof(buf));
}

int
_eina_debug_monitor_service_read(char *op, unsigned char **data)
{
   unsigned char buf[8];
   unsigned int size;
   int rret;

   // read first 8 bytes - payload size (excl size header) with 4 byte
   // opcode that always has to be there
   rret = read(_eina_debug_monitor_service_fd, buf, 8);
   if (rret == 8)
     {
        // store size in int - native endianess as it's local
        memcpy(&size, buf, 4);
        // min size of course is 4 (just opcode) and we will have a max
        // size for any packet of 1mb here coming from debug daemon
        // for sanity
        if ((size >= 4) && (size <= (1024 * 1024)))
          {
             // store 4 byte opcode and guarantee it's 0 byte terminated
             memcpy(op, buf + 4, 4);
             op[4] = 0;
             // subtract space for opcode
             size -= 4;
             // if we have no payload - move on
             if (size == 0) *data = NULL;
             else
               {
                  // allocate a buffer for real payload
                  *data = malloc(size);
                  if (*data)
                    {
                       // get payload - blocking!!!!
                       rret = read(_eina_debug_monitor_service_fd, *data, size);
                       if (rret != (int)size)
                         {
                            // we didn't get payload as expected - error on
                            // comms
                            fprintf(stderr,
                                    "EINA DEBUG ERROR: "
                                    "Invalid Debug opcode read of %i\n", rret);
                            free(*data);
                            *data = NULL;
                            return -1;
                         }
                    }
                  else
                    {
                       // we couldn't allocate memory for payloa buffer
                       // internal memory limit error
                       fprintf(stderr,
                               "EINA DEBUG ERROR: "
                               "Cannot allocate %u bytes for op\n", size);
                       return -1;
                    }
               }
             // return payload size (< 0 is an error)
             return size;
          }
        else
          {
             fprintf(stderr,
                     "EINA DEBUG ERROR: "
                     "Invalid opcode size of %u\n", size);
             return -1;
          }
     }
   fprintf(stderr,
           "EINA DEBUG ERROR: "
           "Invalid opcode read %i != 8\n", rret);
   return -1;
}
#endif
