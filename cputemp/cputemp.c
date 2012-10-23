#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <itemp.h>

void getcputemp()
{
  int fd = -1;
  int err;
  int temp;

  fd = open("/dev/cpu/0/temp", O_RDONLY);
  if (fd == -1) {
    printf("Error opening TEMP: %s (%d)\n", strerror(errno), errno);
    return;
  }

  err = ioctl(fd, X86_GET_CORE_TEMPERATURE, &temp);
  if (err < 0) {
    printf("Error reading temperature: %s (%d)\n", strerror(errno), errno);
  } else {
    printf("CPU temp: %d", temp);
  }

  close(fd);
}

void getmsr(void)
{
  int fd = -1;
  uint32_t buf[2];
  int result;

  fd = open("/dev/cpu/0/msr", O_RDONLY);
  if (fd == -1) {
    printf("Error opening MSR: %s (%d)\n", strerror(errno), errno);
    return;
  }

  result = lseek(fd, 0x19c * 8, SEEK_SET);
  if (result < 0) {
    printf("Error selecting MSR 0x19C: %s (%d)\n", strerror(errno), errno);
    close(fd);
    return;
  }

  result = read(fd, buf, sizeof(buf));
  if (result < 0) {
    printf("Error reading MSR 0x19c: %s (%d)\n", strerror(errno), errno);
    close(fd);
    return;
  }

  printf("MSR, got %d bytes\n", result);
  printf("MSR 0x19C: H=%x, L=%x\n", buf[0], buf[1]);
  close(fd);
}

int main(void) {
  getmsr();
  getcputemp();
}

