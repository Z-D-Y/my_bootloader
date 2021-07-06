/* 执行make时，生成第一个目标boot.bin
 * boot.bin依赖于$(objs)
 * $(objs)中.o文件依赖于.S文件；.o文件依赖于.c文件
 * 就是用对应的命令编译.S、.c文件
 *
 * 编译完之后使用链接命令${OBJCOPY} -O binary -S boot.elf $@
 * 将其链接为elf文件
 *
 * 使用${OBJCOPY} -O binary -S boot.elf $@
 * 将其转换为二进制文件
 * 
 * 使用${OBJDUMP} -D -m arm boot.elf > boot.dis
 * 反汇编
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
