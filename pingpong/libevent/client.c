#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

int64_t total_bytes_read = 0;
int64_t total_messages_read = 0;

void timeoutcb(evutil_socket_t fd, short what, void *arg)
{
  struct event_base *base = arg;
  printf("timeout\n");
  
  event_base_loopexit(base, NULL);
}

static void readcb(struct bufferevent *bev, void *ctx)
{
  /* This callback is invoked when there is data to read on bev. */
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  ++total_messages_read;
  total_bytes_read += evbuffer_get_length(input);

  /* Copy all the data from the input buffer to the output buffer. */
  evbuffer_add_buffer(output, input);
}

void eventcb(struct bufferevent *bev, short events, void *ptr)
{
  if (events & BEV_EVENT_CONNECTED) {
    printf("Connected\n");
  } else if (events & BEV_EVENT_ERROR) {
    printf("NOT Connected\n");
  }
}

int main(int argc, char **argv)
{
  struct event_base *base;
  struct bufferevent *bev;
  struct sockaddr_in sin;
  struct event *evtimeout;
  struct timeval timeout = {5, 0};
  int i;

  int port = 9876;
  int block_size = 16384;
  // int session_count = 10;

  base = event_base_new();
  if (!base) {
    puts("Couldn't open event base");
    return 1;
  }

  char* message = malloc(block_size);
  for (i = 0; i < block_size; ++i) {
    message[i] = i % 128;
  }

  evtimeout = evtimer_new(base, timeoutcb, base);
  evtimer_add(evtimeout, &timeout);

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
  sin.sin_port = htons(port);

  bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

  bufferevent_setcb(bev, readcb, NULL, eventcb, NULL);
  bufferevent_enable(bev, EV_READ|EV_WRITE);
  evbuffer_add(bufferevent_get_output(bev), message, block_size);

  if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    /* Error starting connection */
    bufferevent_free(bev);
    puts("error connect");
    return -1;
  }

  event_base_dispatch(base);

  bufferevent_free(bev);
  event_free(evtimeout);
  event_base_free(base);
  free(message);

  printf("%zd total bytes read\n", total_bytes_read);
  printf("%zd total messages read\n", total_messages_read);
  printf("%.3f MiB/s throughtput\n",
      (double)total_bytes_read / (timeout.tv_sec * 1024 * 1024));
  return 0;
}

