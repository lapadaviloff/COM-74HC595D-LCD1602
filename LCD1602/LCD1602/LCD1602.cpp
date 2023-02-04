/**
* управление LCD1602 с COM порта через сдвиговый регистр 74HC595D
* (TXD->SH_CP, RTS->ST_CP, DTR->DS)
* 
* 03.02.2023
* Mitroshin Aleksey (miam.devsoft@yandex.ru, lapadaviloff@yandex.ru)
*/
#include <bitset>
#include <iostream>
#include <Windows.h>
using namespace std;
HANDLE hSerial;
bool light = 1; // вкыл/выкл подсветка зкрана


void sendByteComPort(byte b);

void comPortIni(const wchar_t* comName);

void displayInit();

void writeChar(const char* data);



int main()
{
	setlocale(LC_ALL, "Russian");
	comPortIni(L"COM6");  //номер COM порта
	displayInit();
	char data[] = "Hello World!"; //отправляемая строка
	writeChar(data);

}

void sendByteComPort(byte b) {

    //сброс DS,ST_CP
	EscapeCommFunction(hSerial, CLRRTS);
	EscapeCommFunction(hSerial, CLRDTR);
	// отправляем побитово в 74HC595D
    for (int i = 0; i < 8; i++) {
		Sleep(1);
        // установка/сброс DS
		if (b & 0b00000001) {
			EscapeCommFunction(hSerial, SETDTR);
		}
		else {
			EscapeCommFunction(hSerial, CLRDTR);
		}
		b = b >> 1;

		char data[] = { 0 };        // отправка SH_CP
		DWORD dwSize = sizeof(data);
		DWORD dwBytesWritten =1; 
	
		BOOL iRet = WriteFile(hSerial, data, dwSize, &dwBytesWritten, NULL);


	}
    // отправка ST_CP
	EscapeCommFunction(hSerial, SETRTS);
	EscapeCommFunction(hSerial, CLRRTS);


}

 void comPortIni(const wchar_t*  comName) {
	LPCTSTR sPortName = comName;

	hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			cout << "serial port does not exist.\n";
			exit(-1);
		}
		cout << "some other error occurred.\n";
	}


	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	
		if (!GetCommState(hSerial, &dcbSerialParams))
		{
		cout << "getting state error\n";
		exit(-1);
		}
	
	dcbSerialParams.BaudRate = CBR_256000;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	
		if (!SetCommState(hSerial, &dcbSerialParams))
		{
		cout << "error setting serial port state\n";
		exit(-1);
		}

}

void displayInit () {
	
	// инициализационные данные из даташит LCD1602 для 4 бит
	// первые 4 бита из даташит, вторые 4 - RS,RW,E, и подсветка
    //    |DATA|RS|RW|E|light|
    //    |0010|0 |0 |1|0    |
    //
	char data[] = {
	0b00100010,
	0b00100010,
	0b11000010,
	0b00000010,
	0b11110010,
	0b00000010,
	0b00010010,
	0b00000010,
	0b01100010
	};
	size_t n = sizeof(data);
	//отправка массива данных
	for (int i = 0; i < n; i++) {
		
		sendByteComPort(data[i]);
		sendByteComPort(0b00000000);

	}
	sendByteComPort(0b00000000 | light);
}

void writeChar(const char* data) {
	//отправка массива данных
	
	byte  H;
	byte  L;
	int i = 0;
	while (data[i] != '\0') {
		//разбиваем байт на старшие м младшие биты
		H = data[i] & 0b11110000;
		L = data[i]  & 0b00001111;
		//    |DATA|RS|RW|E|light|
        //    |H(L)|1 |0 |1|0 (1)|

		sendByteComPort( H | 0b1010 | light); 
		sendByteComPort(0b00001000 | light);

		sendByteComPort(L << 4 | 0b1010 | light);
		sendByteComPort(0b00000000 | light);
		i++;
	}


}