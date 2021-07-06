
/* NAND FLASH������ */
#define NFCONF 				(*((volatile unsigned long *)0x4E000000))
#define NFCONT 				(*((volatile unsigned long *)0x4E000004))
#define NFCMMD 				(*((volatile unsigned char *)0x4E000008))
#define NFADDR 				(*((volatile unsigned char *)0x4E00000C))   /* 2440��ֻ���ĵ�8Ϊ unsigned char����8λ*/
#define NFDATA 				(*((volatile unsigned char *)0x4E000010))
#define NFSTAT 				(*((volatile unsigned char *)0x4E000020))

/* GPIO */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

/* UART registers*/
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)
#define TXD0READY   (1<<2)

void nand_read(unsigned int addr, unsigned char *buf, unsigned int len);
void nand_read0(unsigned int addr, unsigned char *buf, unsigned int len);

extern void puts(char *str);
extern void puthex(unsigned int val);

int isBootFromNorFlash(void)
{
	volatile int *p = (volatile int *)0;
	int val;

	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678) {
		/* д�ɹ�, ��nand���� */
		*p = val;
		return 0;
	} else {
		/* NOR�������ڴ�һ��д */
		return 1;
	}
}

void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)
{	
	int i = 0;
	
	if (isBootFromNorFlash()) { /* �����NOR���� */
		while (i < len) {
			dest[i] = src[i];
			i++;
		}
	} else {	/* �����NAND���� */
		nand_read((unsigned int)src, dest, len);
	}
}

void clear_bss(void)
{
	extern int __bss_start, __bss_end;
	int *p = &__bss_start;
	for (; p < &__bss_end; p++)
		*p = 0;
}

void nand_init(void)
{
    #define TACLS   0
    #define TWRPH0  1
    #define TWRPH1  0
	/* ����ʱ�� */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* ʹ��NAND Flash������, ��ʼ��ECC, ��ֹƬѡ */
	NFCONT = (1<<4)|(1<<1)|(1<<0);	
}

void nand_select(void)
{
    /* NFCONT�ĵ�1ΪΪ0����ʾ����Ƭѡ��Ϊ1��ֹƬѡ 
     * NFCONT�ĵ�0ΪΪ0��nand��������Ϊ1��ʾnand flash controller��������nand init�����ù���
     */
	NFCONT &= ~(1<<1);	
}

void nand_deselect(void)
{
	NFCONT |= (1<<1);	
}

void nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
    /* ��������֮��ȴ�һ�ᣬ���̷����ܻᵼ�·�����ȥ */
	for (i = 0; i < 10; i++);   
}

void nand_addr(unsigned int addr)
{
	unsigned int col  = addr % 2048;    /* �е�ַColumn */
	unsigned int page = addr / 2048;    /* ҳ��ַRow */
	volatile int i;
    /* ȷ�����ʵ�����һ�� */
	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
    /* ȷ�����ʵ�����һҳ */
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);	
}

void nand_page(unsigned int page)
{
	volatile int i;
	
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);	
}

void nand_col(unsigned int col)
{
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
}


void nand_wait_ready(void)
{
    /* NFSTAT�ĵ�0λ
     * Ϊ0����ʾbusy
     * Ϊ1����ʾready
     */
	while (!(NFSTAT & 1));
}

unsigned char nand_data(void)
{
	return NFDATA;
}

int nand_bad(unsigned int addr)
{
	unsigned int col  = 2048;
	unsigned int page = addr / 2048;
	unsigned char val;

	/* 1. ѡ�� */
	nand_select();
	
	/* 2. ����������00h */
	nand_cmd(0x00);
	
	/* 3. ������ַ(��5������) */
	nand_col(col);
	nand_page(page);
	
	/* 4. ����������30h */
	nand_cmd(0x30);
	
	/* 5. �ж�״̬ */
	nand_wait_ready();

	/* 6. ������ */
	val = nand_data();
	
	/* 7. ȡ��ѡ�� */		
	nand_deselect();


	if (val != 0xff)
		return 1;  /* bad blcok */
	else
		return 0;
}


void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = addr % 2048;  /* ��һ�ο��ܲ��Ǵӵ�0�п�ʼ�� */
	int i = 0;
    
	while (i < len) {
		if (!(addr & 0x1FFFF) && nand_bad(addr)) {   /* һ��blockֻ�ж�һ�� */
			addr += (128*1024);  /* ������ǰblock */
			continue;
		}
		/* 1. ѡ�� */
		nand_select();		
		/* 2. ����������00h */
		nand_cmd(0x00);
		/* 3. ������ַ(��5������) */
		nand_addr(addr);
		/* 4. ����������30h */
		nand_cmd(0x30);
		/* 5. �ж�״̬ */
        /* ��Ϊnand�и��ݷ������ĵ�ַ��ҪѰ�����ݣ�
         * �ŵ�Pages registers�ϣ���Ҫʱ�� 
         * ����NFSTAT�Ĵ������ж�״̬
         */
		nand_wait_ready();  
		/* 6. ������ */
        /* �����nand��Pages registers��2048��ǰ2K���ݣ������OOB������Ҫ�� */
		for (; (col < 2048) && (i < len); col++) {
			buf[i] = nand_data();
			i++;
			addr++;
		}
		col = 0;
		/* 7. ȡ��ѡ�� */		
		nand_deselect();
	}
}


#define PCLK            50000000    // init.c�е�clock_init��������PCLKΪ50MHz
#define UART_CLK        PCLK        //  UART0��ʱ��Դ��ΪPCLK
#define UART_BAUD_RATE  115200      // ������
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)
/*
 * ��ʼ��UART0
 * 115200,8N1,������
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3����TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3�ڲ�����

    ULCON0  = 0x03;     // 8N1(8������λ���޽��飬1��ֹͣλ)
    UCON0   = 0x05;     // ��ѯ��ʽ��UARTʱ��ԴΪPCLK
    UFCON0  = 0x00;     // ��ʹ��FIFO
    UMCON0  = 0x00;     // ��ʹ������
    UBRDIV0 = UART_BRD; // ������Ϊ115200
}

/* ����һ���ַ� */
void putc(unsigned char c)
{
    /* �ȴ���ֱ�����ͻ������е������Ѿ�ȫ�����ͳ�ȥ */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* ��UTXH0�Ĵ�����д�����ݣ�UART���Զ��������ͳ�ȥ */
    UTXH0 = c;
}

void puts(char *str)
{
	int i = 0;
	while (str[i]) {
		putc(str[i]);
		i++;
	}
}

void puthex(unsigned int val)
{
	/* 0x1234abcd */
	int i;
	int j;
	
	puts("0x");

	for (i = 0; i < 8; i++) {
		j = (val >> ((7-i)*4)) & 0xf;
		if ((j >= 0) && (j <= 9))
			putc('0' + j);
		else
			putc('A' + j - 0xa);	
	}	
}
