#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdint.h>

struct Request
{
	uid_t uid;
	pid_t pid;
	uint64_t feature;
	int64_t startTime;
};

enum Answer
{
	denied = 0,
	granted = 1
};

#define TRUST_SOCKET "/dev/socket/camera_service/camera_service_to_trust"

int main(int argc, char** argv)
{
	int fd;
	FILE* comm = NULL;
	struct sockaddr_un server;
	int ok;
	struct Request request;
	enum Answer response = granted;
	int ret = 0;
	int flag = 1;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Failed to open stream socket");
		ret = 1;
		goto ret;
	}

	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, TRUST_SOCKET);

	ok = connect(fd, (struct sockaddr*) &server, sizeof(server));
	if (ok < 0) {
		perror("Failed to connect to socket");
		ret = 2;
		goto ret;
	}

	comm = fdopen(fd, "r+");
	if (!comm) {
		perror("Failed to fdopen socket");
		ret = 3;
		goto ret;
	}

	while (read(fd, (void*) &request, sizeof(struct Request)) == sizeof(struct Request)) {
		// Just auto-return permissions to be allowed if asked
		printf("Handling request\n");
		fwrite(&response, sizeof(granted), 1, comm);
		fflush(comm);
		printf("Answered\n");
	}

ret:
	if (comm)
		fclose(comm);
	if (fd >= 0)
		close(fd);
	return 0;
}
