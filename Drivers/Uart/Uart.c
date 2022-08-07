#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "Uart.h"

static u_int8_t s_buf[255]; // バッファ
static ReceiveDataCallback_t s_callback = NULL;
static bool is_load = false;
static u_int8_t s_special_file[16];
static struct termios s_old_tio;
static struct timeval s_tv;

void *ReceiveThread(void *arg);

/**
 * @brief UART受信スレッド
 *
 * @param arg
 * @return void*
 */
void *ReceiveThread(void *arg) {
unload:
  do {
    // デバイスが接続されていない（認識されていない）場合は、ずっとここ。
  } while (is_load == false);
  u_int8_t buf[255]; // バッファ

  int baudRate = B115200;
  int len;

  int fd = open(&s_special_file, O_RDWR);
  if (fd < 0) {
    perror("open");
    printf("%d\n", errno);
    return NULL;
  }

  printf("Port Open\n");

  ioctl(fd, TCGETS, &s_old_tio); // 現在のポートの設定を退避しておく

  struct termios tio;
  tio.c_cflag += CREAD;  // 受信有効
  tio.c_cflag += CLOCAL; // ローカルライン（モデム制御なし）
  tio.c_cflag += CS8;    // データビット:8bit
  tio.c_cflag += 0;      // ストップビット:1bit
  tio.c_cflag += 0;      // パリティ:None

  cfsetispeed(&tio, baudRate);
  cfsetospeed(&tio, baudRate);

  tio.c_cc[VMIN] = 0;
  tio.c_cc[VTIME] = 0;
  tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  cfmakeraw(&tio); // RAWモード

  tcsetattr(fd, TCSANOW, &tio); // デバイスに設定を行う

  ioctl(fd, TCSETS, &tio); // ポートの設定を有効にする

  u_int8_t read_cursor;
  // 送受信処理ループ
  while (1) {
    len = read(fd, buf, 1);
    if (len == 0) {
      goto end;
    }

    if (s_callback != NULL) {
      s_callback(s_buf, len, NULL);
    }

  end:
    if (is_load == false) {
      ioctl(fd, TCSETS, &s_old_tio);
      close(fd);
      printf("Unload\n");
      goto unload;
    }
  }

  return NULL;
}

void Uart_Init(ReceiveDataCallback_t callback) {
  s_callback = callback;
  pthread_t t;
  pthread_create(&t, NULL, ReceiveThread,
                 NULL); // シリアルポートの受信をpthreadで実施します。
}

// デバイスファイルを受け取ります
void Uart_Device_Load(u_int8_t *special_file, u_int32_t len) {
  printf("%s -> ", __func__);
  memcpy(s_special_file, special_file, len);
  s_special_file[len] = '\0';
  for (int i = 0; i < len; i++) {
    printf("%c", s_special_file[i]);
  }
  printf("\n");
  is_load = true;
}

// デバイスが抜かれたことを受け取ります
void Uart_Device_Unload(void) {
  printf("%s\n", __func__);
  is_load = false;
}