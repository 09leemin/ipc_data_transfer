#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#if 0
mqd_t mq_open(const char *name, int oflag);
mqd_t mq_open(const char *name, int oflag, mode_t mode,
              struct mq_attr *attr);
#endif
#define POSIX_MQ_NAME		"/testlip"


#define MSG_STR_SIZE		8192 

static void print_usage(const char *progname)
{
	printf("%s (send PRIORITY |recv)\n", progname);
}

static mqd_t init_msgq(void)
{
	mqd_t mqd;
	struct mq_attr attr;

	mqd = mq_open(POSIX_MQ_NAME, O_RDWR | O_CREAT, 0644, NULL); 
	if(mqd == -1) {
		perror("mq_open()");
		return -1;
	}

	memset(&attr, 0, sizeof(attr));
	if(mq_getattr(mqd, &attr) == -1) {
		close(mqd);
		return -1;
	}

#if 0
	/* struct mq_attr info check */
	printf("mq_flags = %ld\n", attr.mq_flags);
	printf("mq_maxmsg = %ld\n", attr.mq_maxmsg);
	printf("mq_msgsize = %ld\n", attr.mq_msgsize);
	printf("mq_curmsgs = %ld\n", attr.mq_curmsgs);
#endif

	return mqd;
}

static int do_send(unsigned int priority)
{
	int mqd;
	char buf[MSG_STR_SIZE];

	mqd = init_msgq();
	if(mqd == -1) {
		perror("init_msgq()");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "hellow world from pid %d", getpid());

	printf("send msg : %s\n", buf);
	if(mq_send(mqd, buf, sizeof(buf), priority) == -1){
		perror("mq_send()");
		close(mqd);
		return -1;
	}

	return 0;
}

static int do_recv(void)
{
	int mqd;
	char buf[MSG_STR_SIZE];
	unsigned int priority;

	mqd = init_msgq();
	if(mqd == -1) {
		perror("init_msgq()");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	if(mq_receive(mqd, buf, sizeof(buf), &priority) == -1){
		perror("mq_receive()");
		close(mqd);
		return -1;
	}

	printf("prio %u, msg [%s]\n", priority,  buf);

	return 0;
}

int main(int argc, char **argv)
{
	int ret;

	if(argc < 2) {
		print_usage(argv[0]);
		return -1;
	}

	if(!strcmp(argv[1], "send")) {
		unsigned int priority;
		if(argc < 3) {
			print_usage(argv[0]);
			return -1;
		}
		priority = (unsigned int)strtoul(argv[2], NULL, 10);
		ret = do_send(priority);
	}else if(!strcmp(argv[1], "recv")) {
		ret = do_recv();
	}else {
		print_usage(argv[0]);
		return -1;
	}

	return ret;
}
