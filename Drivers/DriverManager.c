#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "DriverManager.h"
#include "Uart.h"
#include "queue.h"

#define GPS_DATA_QUEUE_SIZE 64

typedef struct {
  u_int8_t buf[256];
  u_int32_t len;
} __attribute__((packed)) uart_data_t;

static pthread_mutex_t s_uart_mutex = PTHREAD_MUTEX_INITIALIZER;
static uart_data_t s_uart_data_queue[GPS_DATA_QUEUE_SIZE];
static u_int32_t s_queue_id;
static int s_udev_socket_server;
static int s_udev_socket_server_fd;

static u_int8_t s_udev_event_socket_path[] = "/tmp/udev-socket";
static u_int8_t s_Uart_connect_message[8] = "Add Device!!,";
static u_int8_t s_Uart_disconnect_message[11] = "Remove Device!!,";

void *uart_data_receive_thread(void *arg);

/**
 * @brief Uartドライバからの、パケット受信
 *
 * @param p_data
 * @param len
 * @param p_context
 */
static void ReceiveDataCallback(u_int8_t *p_data, u_int32_t len,
                                void *p_context) {
  pthread_mutex_lock(&s_uart_mutex);
  uart_data_t data;
  memset(data.buf, 0x00, len);
  memcpy(data.buf, p_data, len);
  data.len = len;
  // printf("%s len=%d\n",__func__, data.len);
  queue_enqueue(s_uart_data_queue, (u_int8_t *)&data, s_queue_id);
  pthread_mutex_unlock(&s_uart_mutex);
}

static void uart_packet_check(uart_data_t data) {
  // パケットをよしなに解析
  return;
}

// uartからの受信データを待ち受けるpthread
void *uart_data_receive_thread(void *arg) {
  while (1) {
    pthread_mutex_lock(&s_uart_mutex);
    uart_data_t data;
    if (queue_dequeue(s_uart_data_queue, &data, s_queue_id) != QUEUE_SUCCESS) {
      pthread_mutex_unlock(&s_uart_mutex);
      goto end;
    }
    pthread_mutex_unlock(&s_uart_mutex);
    uart_packet_check(data);

  end:; // Do Nothing.
  }
}

/**
 * @brief
 *
 */
static void initialize(void) {
  int ret = pthread_mutex_init(&s_uart_mutex, NULL);
  queue_entry(s_uart_data_queue, sizeof(uart_data_t), GPS_DATA_QUEUE_SIZE,
              &s_queue_id);
  pthread_t t;
  pthread_create(&t, NULL, uart_data_receive_thread, NULL);
}

/**
 * @brief clientからの接続を待つ
 *
 */
static bool udev_socket_server_connection_wait(void) {
  s_udev_socket_server_fd = accept(s_udev_socket_server, NULL, NULL);
  if (s_udev_socket_server_fd == -1) {
    perror("accept");
    printf("%d\n", errno);
    return false;
  }

  return true;
}

/**
 * @brief Create a udev socket server object
 *
 * @return true
 * @return false
 */
static bool create_udev_socket_server(void) {
  s_udev_socket_server = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s_udev_socket_server == -1) {
    perror("socket");
    printf("%d\n", errno);
    goto error_end;
  }

  // struct sockaddr_un 作成
  struct sockaddr_un sa = {0};
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, s_udev_event_socket_path);

  // 既に同一ファイルが存在していたら削除
  remove(sa.sun_path);

  if (bind(s_udev_socket_server, (struct sockaddr *)&sa,
           sizeof(struct sockaddr_un)) == -1) {
    perror("bind");
    printf("%d\n", errno);
    goto error_end;
  }

  if (listen(s_udev_socket_server, 128) == -1) {
    perror("listen");
    printf("%d\n", errno);
    goto error_end;
  }

  return true;

error_end:
  return false;
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
static bool udev_socket_server_message_receive(void) {
  // 受信
  u_int8_t buffer[1024];
  int recv_size = read(s_udev_socket_server_fd, buffer,
                       sizeof(buffer) - 1); // 受信するまでブロッキング
  if (recv_size == -1) {
    perror("read");
    printf("%d\n", errno);
    close(s_udev_socket_server_fd);
    goto error_end;
  }

  if (strncmp(&buffer[0], s_Uart_connect_message,
              sizeof(s_Uart_connect_message)) == 0) {
    u_int8_t special_file_buf[recv_size - sizeof(s_Uart_connect_message)];
    memcpy(special_file_buf, &buffer[sizeof(s_Uart_connect_message)],
           recv_size - sizeof(s_Uart_connect_message));
    Uart_Device_Load(special_file_buf,
                     recv_size - sizeof(s_Uart_connect_message));
    goto success;
  }

  if (strncmp(&buffer[0], s_Uart_disconnect_message,
              sizeof(s_Uart_disconnect_message)) == 0) {
    Uart_Device_Unload();
    goto success;
  }

success:
  return true;

error_end:
  return false;
}

/**
 * @brief
 *
 */
void main() {
  initialize();

  Uart_Init(ReceiveDataCallback);

  if (!create_udev_socket_server()) {
    return;
  }

  // udevイベントを監視する側からの、ソケット接続を待ち受けます。
  if (!udev_socket_server_connection_wait()) {
    return;
  }

  while (1) {
    if (!udev_socket_server_message_receive()) {
      break;
    }
  }
}