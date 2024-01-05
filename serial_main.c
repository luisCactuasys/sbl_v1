/**
 * @file serial_main.c
 * @author Luís Conceição (luiscarlostc92@gmail.com)
 * @brief   This program open a COM port and reads its content
 * @version 0.1
 * @date 2023-12-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/*
 * *********************** Includes *******************************************
 */
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>


/*
 * *********************** Defines ********************************************
 */

const uint8_t sblACK[2]     = {0x00, 0xCC};
const uint8_t sblNACK[2]    = {0x00, 0x33};
/*
************************ Private Variables ************************************
*/
// UART
HANDLE hSerial;
DCB dcbSerialParams = { 0 };
COMMTIMEOUTS timeouts = { 0 };


/*
 * ********************** Private Function Prototypes **************************
 */
//UART
int32_t uartInit(char *ComPortName);
int32_t uartClose();
int32_t uartSend(uint8_t *buf, uint8_t len);
int32_t uartRecv(uint8_t *buf, uint8_t len);
int32_t uartClear();

//serial bootloader
int32_t sblUartSync();

/*
 * ********************** Function definitions *********************************
 */

/**
 * @brief 
 * 
 * @return int 
 */
int main(int argc, char *argv[]) 
{
    int port = -1;
    int opt;
    char ComPortName[20] = "\\\\.\\COM";
    char forDebugPort[5] = "5";
    int32_t ret = 0;
    

    printf("\nNum of args %d", argc);

    // Using getopt to parse command line options
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -p port_number\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if(!port)
    {
        printf("\nInvalid given port");
        printf("\nOpening default port instead - %s", forDebugPort);
        strcat(ComPortName, forDebugPort);
    }
    else
    {
        strcat(ComPortName, argv[2]);
    }

    uartInit(ComPortName);
    
    
    
    //try to sync UART
    printf("\nSync UART...");
    ret = sblUartSync();
    if(ret == 0 )
    {
        printf("\nSync UART ACK!!");
    }
    else if(ret == 1)
    {
        printf("\nSync UART NACK!!");
    }
    else
    {
        printf("\nSync UART UNKNOWN!!");
    }

    

    
    uartClose();

    return 0;
}





int32_t uartInit(char *ComPortName)
{
    printf("\nOpening Serial Port: %s", ComPortName);
    // Open the serial port
    hSerial = CreateFile(ComPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("\nError: Unable to open serial port");
        return 1;
    }
    printf("\nInit serial port parameters...");
    
    // Set serial port parameters
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("\nError: Unable to get serial port state");
        CloseHandle(hSerial);
        return 1;
    }
    printf("\nChange and Setting serial port parameters...");
    
    dcbSerialParams.BaudRate = 230400;  // Change the baud rate to match your device
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("\nError: Unable to set serial port parameters");
        CloseHandle(hSerial);
        return 1;
    }
    printf("\nSet serial port Read timeouts...");

    // Set read timeouts
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error: Unable to set timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }
    printf("\nSerial port finished opening!");
    return 0;
}

int32_t uartClose()
{
    CloseHandle(hSerial);
    return 0;
}

int32_t uartRecv(uint8_t *buf, uint8_t len)
{
    uint32_t readActualLen = 0;
    ReadFile(hSerial, buf, len, &readActualLen, NULL);
    return 0;
}

int32_t uartSend(uint8_t *buf, uint8_t len)
{
    uint32_t writeActualLen = 0;
    WriteFile(hSerial, buf, len, &writeActualLen, NULL);
    return 0;
}

int32_t uartClear()
{
    PurgeComm(hSerial, PURGE_RXCLEAR);
    return 0;
}




int32_t sblUartSync()
{
    uint8_t uartSync[] = {0x55, 0x55};
    uint8_t recvResp[5] = {0};

    //send bytes to sync
    uartSend(uartSync, sizeof(uartSync));

    //wait for ACK or NACK
    uartRecv(recvResp, 2);

    if(!memcmp(recvResp, sblACK, sizeof(sblACK)))
    {   
        //it was ACK
        return 0;
    }
    else if(!memcmp(recvResp, sblNACK, sizeof(sblNACK)))
    {
        return 1;
    }
    else
    {
        return -1;
    }
    
}
