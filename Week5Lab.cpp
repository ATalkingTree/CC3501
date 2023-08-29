#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <poll.h>

//call it like ./client host port msg

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s port\tListen on the specified port\n", argv[0]);
        return 1;
    }

    /*
    Use getaddrinfo to generate an address structure corresponding to the host
    to connect to.
    */
    struct addrinfo hints;
    struct addrinfo *address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    //hints.ai_flags = AI_PASSIVE; // interpret a NULL hostname as a wildcard (to accept data from anywhere)
    int status = getaddrinfo(argv[1], "3501", &hints, &address); 
    if (status != 0) {
        fprintf(stderr, "Failed to resolve address: %s\n", gai_strerror(status));
        return 1;
    }

    // Open the socket
    int socket_fd = socket(address->ai_family, address->ai_socktype, 0);
    if (socket_fd == -1) {
        // Failed.
        perror("Failed to create socket");
        return 1;
    }

    // Allow broadcast
    int optval = true;
    if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval))) {
        perror("Failed to set SO_BROADCAST");
        return 1;
    }

    // Allow multiple applications to use the same port (to run two versions of the app side by side for testing)
    if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
        perror("Failed to set SO_REUSEPORT");
        return 1;
    }

    // Bind it to the address and port 
    if (0 != bind(socket_fd, address->ai_addr, address->ai_addrlen)) {
        perror("Failed to bind");
        return 1;
    }

    // Prepare the pollfd array with the list of file handles to monitor
    struct pollfd pfds [] = {
        {
            // monitor the socket
            .fd = socket_fd,
            .events = POLLIN | POLLERR,
        },
        // add here if there are other files/sockets to monitor
        {
            // monitor stdin
            .fd = STDIN_FILENO,
            .events = POLLIN | POLLERR,
        }
    };  

    // specify your user name here, either hardcoded or requested
    std::string username = "Louis";
    size_t username_length = username.length() + 1;

    // Event loop
    static char buf [32 + 1 + 240 + 1 + 1];
    for (;;) {
        // Print a prompt
        printf(" > ");
        fflush(stdout); // print the prompt even though it doesn't end

        // Wait for events
        poll(pfds, sizeof(pfds)/sizeof(struct pollfd), -1);

        // Check for keyboard input (i.e. is STDIN_FILENO ready to read)
        if (pfds[1].revents) {
            char keyboard_input[240];
            // read from stdin using read()
            ssize_t message_length = read(STDIN_FILENO, keyboard_input, sizeof(keyboard_input));
            if (message_length < 0) {
                return 0;
            }
            // prepare message to transmit using memcpy()
            memcpy(buf, username.c_str(), username_length);
            memcpy(buf + username_length, keyboard_input, message_length);
            // transmit message on socket using sendto()
            if (-1 == sendto(socket_fd, buf, username_length + message_length, 0, address->ai_addr, sizeof(struct addrinfo))) {
                perror("Failed to send");
                return 1;
            }
        }

        // Check if a packet arrived
        if (pfds[0].revents) {
            // Read the incoming packet
            ssize_t bytes_read = read(socket_fd, buf, sizeof(buf) - 2); // with room for a trailing null
            if (bytes_read < 0) {
                return 0;
            }
            // Make the message null terminated
            buf[bytes_read] = 0;

            // Ensure there is a second null terminated
            buf[bytes_read + 1] = 0;

            // Find a pointer to username and message
            char *username = buf;
            char *message = buf + strlen(buf) + 1;

            // Print it out
            printf("[%s] %s\n", username, message);
        }
    }
    
    // Free the memory returned by getaddrinfo
    freeaddrinfo(address);

    // Close the socket
    close(socket_fd);
}
