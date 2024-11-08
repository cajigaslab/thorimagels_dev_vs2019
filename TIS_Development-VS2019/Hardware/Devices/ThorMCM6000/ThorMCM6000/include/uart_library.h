#pragma once
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the UART_LIBRARY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// UART_LIBRARY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#define UART_LIBRARY_STATIC
#ifdef UART_LIBRARY_STATIC
#define UART_LIBRARY_API extern "C"
#else
#ifdef UART_LIBRARY_EXPORTS
#define UART_LIBRARY_API extern "C" __declspec(dllexport)
#else
#define UART_LIBRARY_API extern "C" __declspec(dllimport)
#endif
#endif

/// <summary>
///  open port function.
/// </summary>
/// <param name="sn">serial number of the device to be opened, use fnUART_LIBRARY_list function to get exist list first.</param>
/// <param name="nBaud">0:hid; 1:usb; others: baudrate value</param>
/// <param name="time">set timeout value in (s)</param>
/// <returns> non-negtive number: hdl number returned successfully; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_open(char *sn, int nBaud, int timeout);

/// <summary>
///  open port function�� with extended parameters
/// </summary>
/// <param name="sn">serial number of the device to be opened, use fnUART_LIBRARY_list function to get exist list first.</param>
/// <param name="nBaud">0:hid; 1:usb; others: baudrate value</param>
/// <param name="time">set timeout value in (s)</param>
/// <param name="databits">Number of bits/byte, 4-8</param>
/// <param name="stopbits">0,1,2 = 1, 1.5, 2</param>
/// <param name="parity">0-4=None,Odd,Even,Mark,Space</param>
/// <param name="xonoff">Enable input X-ON/X-OFF, 0:disable, 1:enable</param>
/// <param name="dtrcontrol">DTR Control Flow Values, 0:disable, 1:enable, 2:handshake</param>
/// <param name="flowcontroll"> RTS Control Flow Values, 0:disable, 1:enable, 2:handshake, 3:toggle</param>
/// <returns> non-negtive number: hdl number returned successfully; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_open_ext(char *sn, int nBaud, int timeout, int databits, int stopbits, int parity, int xonoff, int dtrcontrol, int rtscontrol);

/// <summary>
///  open hid device function.
/// </summary>
/// <param name="sn">serial number of the device to be opened, use fnUART_LIBRARY_list_hid function to get exist list first.</param>
/// <returns> non-negtive number: hdl number returned successfully; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_open_hid(char *sn);

/// <summary>
/// check opened status of port
/// </summary>
/// <param name="sn">serial number of the device to be checked.</param>
/// <returns> 0: port is not opened; 1 : port is opened.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_isOpen(char *sn, int type);

/// <summary>
/// list all the possible port on this computer.
/// </summary>
/// <param name="nPort">port list returned string include serial number and detailed device descriptor, seperated by comma</param>
/// <param name="var">max length value of nPort buffer</param>
/// <returns>non-negtive number: number of device in the list; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_list(char *nPort, int var);

/// <summary>
/// list all the possible hid devices on this computer.
/// </summary>
/// <param name="nPort">device list returned string include serial number and detailed device descriptor, seperated by comma</param>
/// <param name="var">max length value of nPort buffer</param>
/// <returns>non-negtive number: number of device in the list; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_list_hid(char *nPort, int var);

/// <summary>
/// close current opend port
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns> 0: success; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_close(int hdl);

/// <summary>
/// <p>write string to device through opened port.</p>
/// <p>make sure the port was opened successful before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="b">input string</param>
/// <param name="size">size of string to be written.</param>
/// <returns>non-negtive number: number of bytes written; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_write(int hdl, char *b, int size);

/// <summary>
/// <p>read string from device through opened port.</p>
/// <p>make sure the port was opened successful before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="b">returned string buffer</param>
/// <param name="limit">
/// <p>ABS(limit): max length value of b buffer. </p>
/// <p>SIGN(limit) == 1 : wait RX event until time out value expired;</p>
/// <p>SIGN(limit) == -1: INFINITE wait event untill RX has data;</p>
/// </param>
/// <returns>non-negtive number: size of actual read data in byte; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_read(int hdl, char *b, int limit);

/// <summary>
/// <p>set command to device according to protocal in manual.</p>
/// <p>make sure the port was opened successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="c">input command string</param>
/// <param name="var">lenth of input command string (<255)</param>
/// <returns>
/// <p>0: success;</p>
/// <p>0xEA: CMD_NOT_DEFINED;</p>
/// <p>0xEB: time out;</p>
/// <p>0xEC: time out;</p>
/// <p>0xED: invalid string buffer;</p>
/// </returns>
UART_LIBRARY_API int fnUART_LIBRARY_Set(int hdl, char *c, int var);

/// <summary>
/// <p>set command to device according to protocal in manual and get the return string.</p>
/// <p>make sure the port was opened successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="c">input command string (<255)</param>
/// <param name="d">output string (<255)</param>
/// <returns>
/// <p>0: success;</p>
/// <p>0xEA: CMD_NOT_DEFINED;</p>
/// <p>0xEB: time out;</p>
/// <p>0xEC: time out;</p>
/// <p>0xED: invalid string buffer;</p>
/// </returns>
UART_LIBRARY_API int fnUART_LIBRARY_Get(int hdl, char *c, char *d);

/// <summary>
/// this function is not supported yet.
/// </summary>
UART_LIBRARY_API int fnUART_LIBRARY_Req(int hdl, char *c, char *d);

/// <summary>
/// set time out value for read or write process.
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="time">time out value</param>
UART_LIBRARY_API void fnUART_LIBRARY_timeout(int hdl, int time);

/// <summary>
/// Purge the RX and TX buffer on port.
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="flag">
/// <p>FT_PURGE_RX: 0x01</p>
/// <p>FT_PURGE_TX: 0x02</p>
///</param>
/// <returns> 0: success; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_Purge(int hdl, int flag);

/// <summary>
/// get report in byte length for HID device
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns>non-negtive number: number of bytes; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_GET_INPUT_REPORT_LENGTH(int hdl);

/// <summary>
/// get report out byte length for HID device
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns>non-negtive number: number of bytes; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_GET_OUTPUT_REPORT_LENGTH(int hdl);


/// <summary>
/// get report from HID device.
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns>non-negtive number: number of bytes; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_Hid_GetReport(int hdl, char reportId, char* pReport);


/// <summary>
/// set report to HID device.
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns>non-negtive number: number of bytes; negtive number : failed.</returns>
UART_LIBRARY_API int fnUART_LIBRARY_Hid_SetReport(int hdl, char* pReport);