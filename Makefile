/* ִ��makeʱ�����ɵ�һ��Ŀ��boot.bin
 * boot.bin������$(objs)
 * $(objs)��.o�ļ�������.S�ļ���.o�ļ�������.c�ļ�
 * �����ö�Ӧ���������.S��.c�ļ�
 *
 * ������֮��ʹ����������${OBJCOPY} -O binary -S boot.elf $@
 * ��������Ϊelf�ļ�
 *
 * ʹ��${OBJCOPY} -O binary -S boot.elf $@
 * ����ת��Ϊ�������ļ�
 * 
 * ʹ��${OBJDUMP} -D -m arm boot.elf > boot.dis
 * �����
 */

CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

CFLAGS 		:= -Wall -O2
CPPFLAGS   	:= -nostdinc -nostdlib -fno-builtin

objs := start.o init.o boot.o

boot.bin: $(objs)
	${LD} -Tboot.lds -o boot.elf $^
	${OBJCOPY} -O binary -S boot.elf $@
	${OBJDUMP} -D -m arm boot.elf > boot.dis
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.bin *.elf *.dis
