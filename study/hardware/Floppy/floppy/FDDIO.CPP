#ifdef __cplusplus
    #define __CPPARGS ...
#else
    #define __CPPARGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define CYL 0

typedef struct _DPT_
{
  unsigned char srt_hut;
  unsigned char dma_hlt;
  unsigned char motor_w;
  unsigned char sec_size;
  unsigned char eot;
  unsigned char gap_rw;
  unsigned char dtl;
  unsigned char gap_f;
  unsigned char fill_char;
 unsigned char hst;
 unsigned char mot_start;
} DPT;

DPT far *get_dpt(void);
void fdc_out(unsigned char byte);
int  fdc_inp(void);
void int_wait(void);
void dma_init(void far *);
void tdelay(int cnt);
void interrupt IRQ6 (__CPPARGS);
void interrupt (*oldIRQ6) (__CPPARGS);

char buffer[512];

static  int IRQ=0;

void main(void)
{
  unsigned i;
  long l;
  char status[7], main_status;
  DPT _far *fdpt;
  FILE *sect;

  printf("\n\nРабота с контроллером НГМД"
	 "\n  (C)Фролов А., 1995\n");

  oldIRQ6 = _dos_getvect (8+6);
  _dos_setvect (8+6, IRQ6);

  // Открываем файл, в который будем записывать
  // содержимое самого первого сектора дискеты
  sect = fopen("!sector.dat","wb+");

  // Устанавливаем указатель на таблицу
  // параметров дискеты
  fdpt = get_dpt();

  // Включаем мотор дисковода А:
  // Перед этим разрешаем прерывания
  _enable();
  outp(0x3F2, 0x1C);

  // Выполняем задержку для разгона двигателя
  tdelay(18);

  // Показываем содержимое регистра основного
  // состояния контроллера
  printf("Мотор включен.\t\t");
  printf("Основное состояние: %02.2X\n",inp(0x3F4));

  // Перед чтением сектора необходимо установить
  // головку на нужную дорожку, в нашем случае это
  // дорожка с номером CYL

  // Выдаем контроллеру команду "Поиск"
  fdc_out(0xf);

  // Для команды "Поиск" требуется два байта параметров:
  // номер головки/номер накопителя и номер дорожки.
  // Мы работаем с нулевой головкой накопителя А:,
  // поэтому первый параметр равен 0, второй - CYL
  fdc_out(0);
  fdc_out(CYL);

  // Показываем содержимое регистра основного
  // состояния контроллера
  printf("\n<<<Поиск>>> \t\t");
  printf("Основное состояние: %02.2X\n",inp(0x3F4));

  // Ожидаем прерывание по завершению операции
  int_wait();

  // Задержка для позиционирования головки
  tdelay(1);

  // Для проверки результата выполнения команды
  // "Поиск" выдаем контроллеру команду
  // "Чтение состояния прерывания"

  // Выводим содержимое регистра состояния
  // ST0  и номер дорожки после выполнения команды
  // "Поиск" PCN
  fdc_out(0x8);
  printf("Состояние прерывания:\t");
  printf(" ST0: %02.2X, \t", fdc_inp());
  printf("PCN: %02.2X\n", fdc_inp());

  // Для более глубокой диагностики состояния
  // контроллера выдаем контроллеру команду
  // "Чтение состояния накопителя", выводим
  // содержимое регистра состояния ST3
  fdc_out(4);
  fdc_out(0);
  printf("Состояние накопителя:\t ST3: %02.2X\n",fdc_inp());

  // Устанавливаем скорость передачи данных 500 Кбайт/с
  outp(0x3F7, 0);

  // Инициализация канала прямого
  // доступа к памяти
  dma_init((void far *)buffer);

  // Выдаем команду "Чтение данных"
  fdc_out(0x66);
  fdc_out(0x0);     // накопитель 0, головка 0

  fdc_out(CYL);     // цилиндр CYL
  fdc_out(0);       // головка 0
  fdc_out(1);       // номер сектора - 1

  // Передаем контроллеру технические параметры
  // дисковода, берем их из таблицы параметров дискеты.
  // Это такие параметры:
  //    - размер сектора;
  //    - номер последнего сектора на дорожке;
  //    - размер промежутка;
  //    - число считываемых/записываемых байтов
  fdc_out(fdpt->sec_size);
  fdc_out(fdpt->eot);
  fdc_out(fdpt->gap_rw);
  fdc_out(fdpt->dtl);

  // Ожидаем прерывание после завершения операции
  int_wait();

  // Считываем и выводим на экран байты результата
  // операции "Чтение данных"
  printf("\n<<<Чтение сектора>>> \n");
  printf("   Байты состояния (ST0,ST1,ST2,C,H,R,N):\n");

  for(i=0; i<7; i++) printf("%02.2X\t", (char) fdc_inp());
  printf("\n");

  // Выводим содержимое считанного сектора в файл
  for(i=0; i<512; i++) fputc(buffer[i],sect);
  fclose(sect);

  // Выключаем мотор
  outp(0x3F2, 0xC);

  _dos_setvect (8+6, oldIRQ6);
}

// Вывод байта в контроллер дисковода
void fdc_out(unsigned char parm)
{
  asm mov   dx,3F4h     // Порт основного состояния
loop_fdc_out:

  asm in    al,dx
  asm test  al,80h      // Проверяем готовность
  asm jz loop_fdc_out   //   контроллера

  asm inc   dx          // Выводим байт в порт данных
  asm mov   al, parm    //   контроллера
  asm out   dx, al
}

// Ввод байта из порта данных контроллера дисковода
int fdc_inp(void)
{
  asm mov   dx,3F4h     // Порт основного состояния
loop_fdc_inp:
  asm in    al,dx
  asm test  al,80h      // Проверяем готовность
  asm jz loop_fdc_inp   //   контроллера

  asm inc   dx          // Введенный байт записываем
  asm in    al, dx      // в регистр AX
  return _AL;
}

// Ожидание прерывания от контроллера
void int_wait (void) {
  _enable();
  while (IRQ==0) {};
  IRQ = 0;
}

void interrupt IRQ6 (__CPPARGS) {
  IRQ = 1;
  outportb (0x20, 0x20);
}

// Инициализация канала прямого доступа к памяти
void dma_init(void far *buf)
{
  unsigned long f_adr;
  unsigned sg, of;

  // Вычисляем 24-разрядный адрес буфера для данных
  f_adr = ((unsigned long)FP_SEG(buf) << 4)
    + (unsigned long)FP_OFF(buf);

  // Расщепляем адрес на номер страницы
  // и смещение
  sg = (f_adr >> 16) & 0xff;
  of = f_adr & 0xffff;

  // На время программирования контроллера прямого
  // доступа запрещаем прерывания
  _disable();
  asm mov   al,46h   // Команда чтения данных от
                     // контроллера НГМД

  asm out   12,al    // Сброс триггера-указателя байта
                     // для работы с 16-разрядными портами.
		     // Следующий байт, выводимый в 16-разрядный
		     // порт будет интерпретироваться
		     // как младший

  asm out   11,al    // Установка режима контроллера ПДП

  asm mov   ax,of    // Смещение буфера, младший байт
  asm out   4,al
  asm mov   al,ah    // Смещение буфера, старший байт
  asm out   4,al

  asm mov   ax,sg    // Номер страницы
  asm out   81h,al

  asm mov   ax,511   // Длина передаваемых данных
  asm out   5,al
  asm mov   al,ah
  asm out   5,al

  asm mov   al,2     // Разблокировка канала 2 контроллера ПДП
  asm out   10,al

  // Инициализация контроллера закончена,
  // разрешаем прерывания.
  _enable();
}

/**
* get_dpt
*
* Вычислить адрес таблицы параметров дискеты
*
* Функция возвращает указатель на таблицу
* параметров дискеты
*
**/

DPT far *get_dpt(void)
{
  void far * far *ptr;

  ptr = (void far * far *)MK_FP(0x0, 0x78);
  return(DPT far*)(*ptr);
}

/**
* tdelay
*
* Формирование временной задержки при помощи
* таймера.
*
* В качестве параметра функции передается
* длительность задержки в количестве прерываний,
* поступающих от таймера (таймер генерирует
* в одну секунду примерно 18 прерываний)
*
**/

void tdelay(int cnt)
{
  asm push bx
  asm push dx
  asm push si

  asm mov si, cnt
  asm mov ah, 0
  asm int 1ah
  asm mov bx, dx
  asm add bx, si

delay_loop:
  asm int 1ah
  asm cmp dx, bx
  asm jne delay_loop

  asm pop si
  asm pop dx
  asm pop bx
}
