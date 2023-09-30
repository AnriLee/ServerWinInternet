#ifndef ObgectH
#define ObgectH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>

struct SEN { float data; unsigned char flag; }; // Данные датчиков
struct POR { float Pmin; float Pmax; };
struct FLAG // настроечные данные флагов
	{
	unsigned short FlagDevise;
	unsigned short FlagBit;
	};
struct TIMER // настроечные данные таймеров
	{
	unsigned short TimerArh; // таймер архивации данных (в мин.)
	unsigned short TimerCon; // таймер выхода на связь (в мин.)
	};
struct KORDINATE
	{
	float Lat; // Широта
	float Lon; // долгота
	};

// описание устройства -> настроечные данные устройств GRP и KZ (BLOB)
	//FlagDevise;
	// 1 - U (v) AK, 2 - Pin, 4 - Pout, 8 - Gaz
	// 10 - PSK1, 20 - T1, 40 - T2, 80 - PKZ
	// 100 - P2, 200 - PSK2, 400 - вн. AK
	// 1000 - IKZ, 2000 - UKZ
	// 4000 - U ~220
	// 800 - импульсный счётчик газа
	//FlagBit;
	// 1 - PSK1, 2 - PZK1, 4 - DV1, 8 - DV2, 10 - DV3,
	// 20 - DV4, 40 - Regl., 80 - Test, 100 - Pauza
	// 200 - PZK2, 400 - PSK2 ,
	// 0x2000 - включение Delta, 0x4000 - включение доп. порогов
	// 8000 - U питания
struct PARAMETRS_TERMINAL
	{
	char HostServer[30]; // адрес сервера
	unsigned short PortServer; // порт сервера
	char Number[20]; // имя объекта (номер)
	struct KORDINATE KordinateDevise;
	struct FLAG Flag; // настроечные данные флагов
	struct TIMER Timers; // настроечные данные таймеров
	unsigned short lPowerTimer; // период обработки мало мощных датчиков (в мин.)
	unsigned short hPowerTimer; // период обработки мощных датчиков (в мин.)
	struct POR DevPor[16]; // все датчики с АК
	unsigned char DopPor;
	};
//==============================================================================

struct TERMINAL_COUNTER // настроечные данные
	{
	char HostServer[30]; // адрес сервера
	unsigned short PortServer; // порт сервера
	char Number[20]; // имя объекта
	struct FLAG Flag; // настроечные данные флагов
	struct TIMER Timers; // настроечные данные таймеров
	unsigned char SetCount; // 1 - записать счётчик
//	struct ParCntGaz ParCounter;
	};

union SETDEV // внутреннее описание всех устройства
	{
	struct PARAMETRS_TERMINAL ParametrsGRP;
	struct TERMINAL_COUNTER ParametrsCounter;
	};

/*************************************************************************/
/****************** Структуры обмена с устройствами **********************/
/*************************************************************************/

struct PUBLIC // шапка любой команды
	{
	char Password[6]; // "ORION"
	unsigned char Command;
					// 1 - данные с архива, 2 - последний пакет
					// 4 - пакет настройки, 5 - пакет времени, 10 - пакет отбоя
					// 20 - пакет памяти FLESH
					// 21 - пакет памяти EEP
					// 22 - прошить память FLASH
					// 23 - прошить память EEP
					// 24 - прошить память FLASH & EEP
	unsigned char TypeDevise; // тип устройства (0x40 - GRP internet)
	char ID[20]; // ID объекта
	char Number[20]; // имя объекта
	unsigned char VerDevise; // версия устройства
	struct KORDINATE KordinateDevise;
	unsigned char Date[6]; // Время (Date[2]-год & 0x80 -> по Гривничу )
	unsigned short NamberPacked; // номер пакета (0xFFFF - последний пакет)
	unsigned char PackedLong; // длина пакета
	};

struct CntGazCSD
	{
	char Flag;
	float RoutData;  // Текущее значение
	float HourData;	 // Текущее часовое значение
	float AllData;	 // Общий расход
	};

struct ARHIV_TERMINAL
	{
	unsigned char constCSQ;
	struct FLAG FlagSetup;
	struct FLAG FlagError;
	struct SEN Sen[16];
	};

struct ORION // контрольный пакет для ORION (3 команда)
	{
	char Host[30]; // адрес сервера
	short PortServer; // порт сервера
	unsigned char TypeDevise; // тип устройства (0x42 - катодная защита (internet))
	unsigned char VerDevise; // версия устройства
	struct KORDINATE KordinateDevise;
	struct FLAG Flag; // настроечные данные флагов
	struct TIMER Timers; // настроечные данные таймеров
	unsigned short lPowerTimer; // период обработки мало мощных датчиков (в мин.)
	unsigned short hPowerTimer; // период обработки мощных датчиков (в мин.)
	struct POR DevPor[16];
	unsigned char DopPor;
	};

struct INP_ARH
	{
	struct PUBLIC Pbl;// шапка любой команды
	union { struct ARHIV_TERMINAL Arh; struct ORION Param; };
	};

struct OUT_PARAM_TERMINAL
	{
	struct PUBLIC Pbl; // шапка любой команды
	union SETDEV Data; // внутреннее описание всех устройства
	};

/*************************************************************************/
/************************ Команды обмена програм *************************/
/*************************************************************************/

struct COM // общая для всех команд
	{
	char Password[6];
	char Command[10];
		// 0 - Тест
		// 1 - запрос данных сервера
		// 2 - запрос данных устройств
		// 3 - получение текущих данных
		// 4 - запись (удаление) данных устройства
		// 5 - сделать звонок для вызова
		// 6 - диапазон архива
		// 7 - данные архива устройств
		// 8 - получить параметры ожидающие настройки
		// 9 - данные архива диспетчера
		// 10 - вход в программу пользователя
		// 11 - выход из программы пользователя
		// 12 - любые приходящие данные
		// 13 - включить WEB сервер
		// 14 - выключить WEB сервер
	char Name[50]; // имя пользователя
	unsigned char Fun;
	unsigned int PackedLong; // длина пакета
//   char Data[...]; // структура зависит от принимаемых данных
   };

 struct GETDATA
	{
	struct COM Comand;
	// struct OLDDATA.....
	};

 struct SETDATA
	{
	struct COM Comand;
	// struct OUT_ARHI.....
	};

 struct DEV // информация о приборе
	{
	char ID[20]; // ID прибора
	char Code[20]; // (номер) прибора
	};

 struct NEWPAR
	{
	char FlagLoad; // 0-нет данных, 1-данные получены
	unsigned char TypeDevise;
	struct DEV ID;
	union SETDEV Parametrs; // пакет настройки устройства
	};

 struct INFO // информация общего пользования (потверждение аварии)
	{
	unsigned short Number; // номер объекта
	char Date[6];  // время аварии
	};

/****************************************************************/
//================== ПОЛЯ ТАБЛИЦ БАЗЫ ДАННЫХ  ====================
/****************************************************************/

struct SERVERINFO // настройка сервера
	{
	char HostServer[50]; // IP или Host сервера для настройки обьектов
	int ExtPortServer; // Порт сервера для настройки обьектов (external внешний)
	int IntPortServer; // Локальный порт сервера (interior внутренний)
	char AdminPassword[30];
	int TimerSaveDB; // время хранения данных (в днях)
	char HostInsert[50]; // IP или Host сервера изменения данных
	char NameInsert[50]; // имя пользователя изменения данных
	double TimerInsert; // время изменения данных
	};

struct TERMINALINFO // информация о приборе
	{
	unsigned int TabNo;
	char ID[20]; // ID объекта (не меняется)
	char Code[20]; // имя объекта
	unsigned char VerDevise; // версия устройства
	char SaveFlag; // флаг изменения данных (default 0)
	double TimerInsert; // время изменения данных
	char HostInsert[50]; // IP или Host сервера изменения данных
	char NameInsert[50]; // имя пользователя изменения данных
	unsigned char TypeDevise;
							// 0..0xF - тип устройства
							// 0x00 - стандартное устройство
							// 0x1 - SENSOR
							// 0x2 - стандартное устройство катодки
							// 0x3 - счётчик
							// 0xF0 - состояние устройства
							// 0x10 - прибор работает по GSM (1)
							// 0x20 - прибор работает по GPRS (1)
							// 0x40 - прибор работает по Internet
	char Name[80];
	char Region [50]; // область
	char DistrictCity [50]; // район
	char Addres[80];
	struct KORDINATE KordinateDevise;
	// внутреннее описание всех устройства -> настроечные данные (BLOB)
	union SETDEV Parametrs; // приходящий пакет настройки устройства
	};

// поля TERMINAL_ARHIV // Таблица архивных данных
 // ID (номер) прибора
 // TimerOut время измерения данных
 // TimerInp время получения данных
 // constCSQ уровень связи
 // SensorError флаг ошибок датчиков
 // BitError флаг ошибок битовых полей
 // --- датчики ---
 // 0 - AK  Внутрений акумулятор (0x1)
 // 1 - P_1 Давление Р1 (0x2), 2 - P_2  Давление Р2 (0x100), 3 - P_3  Давление Р3 (0x4)
 // 4 - PSK_1  ПСК 1 (0x10), 5 - PSK_2  ПСК 2 (0x200)
 // 6 - CnHm_1  Загазованность (0x8)
 // 7 - T_1  Температура 1 (0x20), 8 - T_2  Температура 2 (0x40)
 // 9 - Ektz  Катодная защита (0x80)
 // 10 - `U_220` ~220
 // 11 - Rez_AK  Внешний акумулятор (0x400)
 // --- битовые поля ---
 // PZK_1, ПЗК 1, PZK_2, ПЗК 2
 // DV_1 Дв. 1, DV_2 Дв. 2, DV_3 Дв. 3, DV_4 Дв. 4
struct OUT_ARHIV // Таблица архивных данных
	{
	char ID[20]; // ID прибора
	char Code[20]; // (номер) прибора
	double TimerOut; // время измерения данных
	double TimerInp; // время получения данных
	unsigned char constCSQ; // уровень связи
	struct FLAG FlagSetup;
	struct FLAG FlagError;
	struct SEN Sen[16];
	};

extern struct SERVERINFO Server; // настройка сервера
#endif

