#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>

unsigned int delayTime = 0;

char date[6];

unsigned int registers[] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09 }; 
/*00h – секунды
01h – секунды будильника
02h – минуты
03h – минуты будильника
04h – часы
05h – часы будильника
06h – день недели
07h – день месяца
08h – месяц
09h – год две младшие две цифры
*/
void interrupt(*oldTimer)(...);
void interrupt(*oldAlarm) (...);

void getTime();
void setTime();
void delay(unsigned int);
void setAlarm();
void resetAlarm();
void inputTime();
int decToBcd(int);

void interrupt newTimer(...)
{
	delayTime++;

	outp(0x70, 0x0C);	//Если регистр C не будет прочитан после IRQ 8, 
						//тогда прерывание не случится снова
	inp(0x71);

	outp(0x20, 0x20);//Посылаем контроллеру прерываний (master)сигнал EOI (end of interrupt)
	outp(0xA0, 0x20); // Посылаем второму контроллеру прерываний (slave) сигнал EOI (end of interrupt)
}

void interrupt newAlarm(...)
{
	puts("Alarm");

	/////////////////////////
	oldAlarm(); //вызвать ещё и обычное прерывание, которое вызываеся при срабатывании будильника
	resetAlarm();
}

int main()
{
	int delayMillisecond;
	while (1)
	{
		printf("1 - Current time\n");
		printf("2 - Set time\n");
		printf("3 - Set alarm\n");
		printf("4 - Set delay\n");

		switch (getch()) 
		{
		case '1':
		{
			getTime();
			break;
		}
		case '2':
		{
			setTime();
			break;
		}
		case '3':
		{
			setAlarm();
			break;
		}
		case '4':
		{
			fflush(stdin);
			printf("Input delay in millisecond: ");
			scanf("%d", &delayMillisecond);
			delay(delayMillisecond);
			break;
		}
		default:
		{
			system("cls");
			break;
		}
		}
	}
}

void getTime()
{
	system("cls");

	int i = 0;
	for (i = 0; i < 6; i++)
	{
		outp(0x70, 0x0B);
		outp(0x71, inp(0x71)|4);//4 - 3 бит поставить в единицу
		outp(0x70, registers[i]);//индекс выбора регистра CMOS (установка на нужную ячейку)
		date[i] = inp(0x71);//порт данных (чтение из ячейки)
	}

	printf("Time: %d:%d:%d\n", date[2], date[1], date[0]);
	printf("Date: %d.%d.%d\n", date[3], date[4], date[5]);
}

void setTime()
{
	inputTime();
	disable(); //чтобы во время работы не вызвалось прерывание и не сломалось время

	unsigned int res;
	do
	{

		outp(0x70, 0xA); //работа с срабатыванием прерываний
		res = inp(0x71) & 0x80; //80 это 1-000-0000, т.е. обновление идёт
		/*
		0Ah – Регистр A задает частоту срабатывания прерывания.
			Бит 7 - обновление времени 1 идет обновление 0 обновление еще не началось.
			Биты 6 - 4 – управление делителем частоты
			010 = Нормальный режим
			11X = Сброс делителя, его перезагрузка(reset)
			101 = Пропустить 15 периодов(test mode only)
			100 = Пропустить 10 периодов(test mode only)
			011 = Пропустить 5 периодов(test mode only)
			001 = Invalid
			000 = Invalid
			Биты 3 - 0 –выходная частота делителя
			0000 = Без прерываний
			0001 = 3, 90625 мс 256Гц
			0010 = 7, 8125 мс 128Гц
			0011 = 122, 070 мкс 8192 Гц
			0100 = 244, 141 мкс 4096 Гц
			0101 = 488, 281 мкс 2048 Гц
			0110 = 976, 5625 мкс 1024 Гц - стандартная
			0111 = 1, 953125 мс 512 Гц
			1000 = 3, 90625 мс 256 Гц
			1001 = 7, 8125 мс 128 Гц
			1010 = 15, 625 мс 64 Гц
			1011 = 31, 25 мс 32 Гц
			1100 = 62, 5 мс 16 Гц
			1101 = 125 мс 8 Гц
			1110 = 250 мс 4 Гц
			1111 = 500 мс 2 Гц
			*/
	} while (res); //дождаться окончания обновления

	outp(0x70, 0xB); //работа с часами
	outp(0x71, inp(0x71) | 0x80);	//сделать 7 бит единицей (остановить часы),
									//а остальное оставить как было
	/*0Bh - регистр B
		Бит 7 остановка часов.
		1 Часы остановлены
		0 Нормальный ход.
		Бит 6 PIE разрешение периодических прерываний
		1 - разрешено
		0 - запрещено
		Бит 5 AIE Разрешить генерировать прерывание от будильника
		1 - при достижение времени будильника будет вызвано прерывание.
		0 - не генерировать прерывание при срабатывании будильника.
		Бит 4 UIE разрешить прерывание по окончанию смены времени
		1 - разрешено
		0 - запрещено
		Бит 3 разрешить выдавать сигнал меандра, с частотой заданной регистром A.
		1 - разрешено
		0 - запрещено
		Бит 2 Задает режим данных BCD / BIN для регистров 0 - 9
		1 BIN
		0 BCD
		Бит 1 Режим время исчисления 12 / 24
		1 - 12
		0 - 24
		Бит 0 Выполнять переключение на зимнее / летние время.
		1 переключение разрешено.
		0 переключение запрещено
		*/
	for (int i = 0; i < 3; i++) //Возможно здесь 4!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	{
		outp(0x70, registers[i]);
		outp(0x71, date[i]);
	}

	outp(0x70, 0xB); //работа с часами
	outp(0x71, inp(0x71) & 0x7F);	//обнулить только 7й бит (включить нормальный 
									//ход часов), а остальное оставить как было
	enable(); //включение работы прерываний
	system("cls");
}

void delay(unsigned int ms)
{
	disable(); //чтобы во время работы не вызвалось прерывание и не сломалось время

	oldTimer = getvect(0x70);
	setvect(0x70, newTimer); //перезапись прерывания на новое значение

	enable();

	outp(0xA1, inp(0xA1) & 0xFE);	//1111-1110 - обнуление нулевого бита регистра
									//данных ведомого контроллера прерываний
									//чтобы разрешить прерывания от ЧРВ
									// 0й порт это кмоп часы реального времени
									// или высокоточный таймер событий

	outp(0x70, 0xB); //работа с часами
	outp(0x71, inp(0x71) | 0x40);	// 0100-0000
									//разрешение периодических прерываний
	delayTime = 0;
	while (delayTime <= ms);

	puts("Delay's end");
	setvect(0x70, oldTimer);
	return;
}

void setAlarm()
{
	inputTime();

	disable(); //чтобы во время работы не вызвалось прерывание и не сломалось время

	unsigned int res;
	do
	{
		outp(0x70, 0xA);//regisrty a -- частота срабатывания прерывания
		res = inp(0x71) & 0x80;//80 это 1-000-0000, т.е. обновление идёт
		
	} while (res); //дождаться окончания обновления часов

	outp(0x70, 0x05);
	outp(0x71, date[2]);

	outp(0x70, 0x03);
	outp(0x71, date[1]);

	outp(0x70, 0x01);
	outp(0x71, date[0]);

	outp(0x70, 0xB); //работа с часами
	outp(0x71, (inp(0x71) | 0x20));//0-0-1-0-0-0-0-0 вызвать прерывание при срабатывании будильника

	oldAlarm = getvect(0x4A); //0х4А - обработчик прерывания будильника
	setvect(0x4A, newAlarm); //перезапись прерывания на кастомное

	outp(0xA1, (inp(0xA0) & 0xFE)); ///Наверное 1111-1110 - обнуление нулевого бита регистра
									///данных ведомого контроллера прерываний
									///чтобы разрешить прерывания от ЧРВ
	enable();
	printf("Alarm enabled\n");
}

void resetAlarm()
{
	if (oldAlarm == NULL) //значит просто рандомное срабатывание, надо игнорить
	{
		return;
	}

	disable();

	setvect(0x4A, oldAlarm);
	outp(0xA1, (inp(0xA0) | 0x01)); //0000-0001 Устанавливаем последний бит в 1 НАХУЯ?????

	unsigned int res;
	do
	{
		outp(0x70, 0xA);
		res = inp(0x71) & 0x80;
	} while (res);  //дождаться окончания обновления часов

	outp(0x70, 0x05);//часы будильника
	outp(0x71, 0x00);

	outp(0x70, 0x03);//минуты будильника
	outp(0x71, 0x00);

	outp(0x70, 0x01);//секунды будильника
	outp(0x71, 0x00);

	outp(0x70, 0xB); //работа с часами
	outp(0x71, (inp(0x71) & 0xDF)); //1101-1111 Запретить генерировать прерывание от будильника
	enable();
}

void inputTime()
{
	int n;

	do {
		fflush(stdin);
		printf("Input hours: ");
		scanf("%i", &n);
	} while ((n > 23 || n < 0));
	date[2] = n;

	do {
		fflush(stdin);
		printf("Input minutes: ");
		scanf("%i", &n);
	} while (n > 59 || n < 0);
	date[1] = n;

	do {
		fflush(stdin);
		printf("Input seconds: ");
		scanf("%i", &n);
	} while (n > 59 || n < 0);
	date[0] = n;
}

int decToBcd(int dec)
{
	return ((dec / 10 * 16) + (dec % 10));
}
