#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <errno.h>

typedef struct _EBPB_
{
  unsigned sectsize;
  char clustsize;
  unsigned ressecs;
  char fatcnt;
  unsigned rootsize;
  unsigned totsecs;
  char media;
  unsigned fatsize;
  unsigned seccnt;
  unsigned headcnt;
  unsigned hiddensec_low;
  unsigned hiddensec_hi;
  unsigned long drvsecs;
} EBPB;

typedef struct _TRK_LY_
{
  unsigned no;
  unsigned size;
} TRK_LY;

typedef struct _DPB_
{
  char spec;
  char devtype;
  unsigned devattr;
  unsigned numofcyl;
  char media_type;
  EBPB bpb;
  char reserved[6];
  unsigned trkcnt;
  TRK_LY trk[100];
} DPB;

typedef struct _DPB_FORMAT_
{
  char spec;
  unsigned head;
  unsigned track;
} DPB_FORMAT;

int main(void)
{
  union REGS reg;
  struct SREGS segreg;
  DPB far *dbp;
  DPB_FORMAT far *dbp_f;
  int sectors, i;

  printf("\nПрограмма уничтожит содержимое"
   "\n81-й дорожки диска А:."
   "\nЖелаете продолжить? (Y,N)\n");

  i = getch();
  if((i != 'y') && (i != 'Y'))
    return(-1);

  // Заказываем память для блока параметров устройства
  dbp = (DPB far*)farmalloc(sizeof(DPB));

  // Заказываем память для блока параметров устройства,
  // который будет использован для форматирования
  dbp_f = (DPB_FORMAT far*)
    farmalloc(sizeof(DPB_FORMAT));

  if(dbp == NULL || dbp_f == NULL)
  {
    printf("\nМало памяти");
    return(-1);
  }

  // Получаем текущие параметры диска А:
  dbp->spec = 0;
  reg.x.ax = 0x440d;
  reg.h.bl = 1;
  reg.x.cx = 0x0860;
  reg.x.dx =  FP_OFF(dbp);
  segreg.ds = FP_SEG(dbp);
  intdosx(&reg, &reg, &segreg);

  if(reg.x.cflag != 0)
  {
    printf("\nОшибка: %d", reg.x.ax);
    return(-1);
  }

  // Заполняем блок парметров для форматирования
  dbp->spec = 5;

  // Считываем из BPB количество секторов на дорожке
  sectors = dbp->bpb.seccnt;

  // Подготавливаем таблицу, описывающую формат дорожки

  // Записываем количество секторов на дорожке
  dbp->trkcnt = sectors;

  // Для каждого сектора на дорожке в таблицу
  // записываем его номер и размер.
  for(i = 0; i < sectors; i++)
  {
    dbp->trk[i].no   = i+1;
    dbp->trk[i].size = 512;
  }

  // Устанавливаем новые параметры для диска А:
  reg.x.ax = 0x440d;
  reg.h.bl = 1;
  reg.x.cx = 0x0840;
  reg.x.dx =  FP_OFF(dbp);
  segreg.ds = FP_SEG(dbp);
  intdosx(&reg, &reg, &segreg);

  if(reg.x.cflag != 0)
  {
    printf("\nОшибка: %d", reg.x.ax);
    return(-1);
  }

  // Готовим блок параметров устройства,
  // который будет использован при вызове
  // операции проверки возможности форматирования
  // дорожки
  dbp_f->spec = 1;
  dbp_f->head = 0;
  dbp_f->track = 81;

  reg.x.ax = 0x440d;
  reg.h.bl = 1;
  reg.x.cx = 0x0842;
  reg.x.dx =  FP_OFF(dbp_f);
  segreg.ds = FP_SEG(dbp_f);
  intdosx(&reg, &reg, &segreg);

  if(reg.x.cflag != 0)
  {
    printf("\nОшибка: %d", reg.x.ax);
    return(-1);
  }

  // Если указанный формат дорожки поддерживается,
  // поле специальных функций будет содержать 0.
  // Проверяем это
  if(dbp_f->spec != 0)
  {
    printf("\nФормат дорожки не поддерживается");
    return(-1);
  }

  // Заполняем блок параметров для выполнения
  // операции форматирования
  dbp_f->spec = 0;
  dbp_f->head = 0;
  dbp_f->track = 81;

  // Форматируем дорожку с номером 81, головка 0
  reg.x.ax = 0x440d;
  reg.h.bl = 1;
  reg.x.cx = 0x0842;
  reg.x.dx =  FP_OFF(dbp_f);
  segreg.ds = FP_SEG(dbp_f);
  intdosx(&reg, &reg, &segreg);

  if(reg.x.cflag != 0)
  {
    printf("\nОшибка: %d", reg.x.ax);
    return(-1);
  }

  // Освобождаем память
  farfree(dbp);
  farfree(dbp_f);

  return(0);
}

