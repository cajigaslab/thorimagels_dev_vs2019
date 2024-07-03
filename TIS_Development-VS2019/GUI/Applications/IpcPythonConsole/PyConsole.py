import io
import msvcrt as ms # for fd magic

import win32api, win32file, win32pipe #used for output pipe (TX)

from multiprocessing import Process #used for running this programs client(RX)

import socket #used for getting host PC Name

import time
import sys
import win32pipe, win32file, pywintypes

import msvcrt

#Returns True if client should be torn down
def print_client_cmd(cmd: str):
    src = cmd.split('~')[0]
    dst = cmd.split('~')[1]
    cmdName = cmd.split('~')[2]
    payload = cmd.split('~')[3]
    
    cmdName = cmdName.replace('\x00', '')
    payload = payload.replace('\x00', '')
    if cmdName == 'PositionReportZ':
        print('Z stage position: '+payload + '[um]')
    elif cmdName == 'PositionReportX':
        print('X stage position: '+payload + '[um]') 
    elif cmdName == 'PositionReportY':
        print('Y stage position: '+payload + '[um]')
    elif cmdName == 'PositionReportSecondaryZ':
        print('Secondary Z stage position: '+payload + '[um]')
    elif cmdName == 'TearDown':
        print('Tearing down')
        return True
    return False 

#get an input key
def get_key():
    """Get a key press from the user."""
    key = msvcrt.getch()  # Get a key press
    key = key.decode() if isinstance(key, bytes) else key  # Decode bytes to string if necessary
    return key

#send a command to external client
def pipe_out(inpStr: str):
    pipe = win32pipe.CreateNamedPipe(
        r'\\.\pipe\ConsoleTestThorImagePipe',
        win32pipe.PIPE_ACCESS_DUPLEX,
        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
        4, 65536*2, 65536*2,
        0,
        None)
    try:
        win32pipe.ConnectNamedPipe(pipe, None)
        
        some_data = str.encode(inpStr, encoding='utf-16')
        win32file.WriteFile(pipe, some_data)
    finally:
        win32file.CloseHandle(pipe)

#establish connection with the external client
def pipe_server():
    pipe = win32pipe.CreateNamedPipe(
        r'\\.\pipe\ConsoleTestThorImagePipe',
        win32pipe.PIPE_ACCESS_DUPLEX,
        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
        1, 65536, 65536,
        0,
        None)
    try:
        win32pipe.ConnectNamedPipe(pipe, None)
        PCName = socket.gethostname()
        some_data = str.encode("Remote~Local~Establish~" + PCName, encoding='utf-16') #STL-RTEER-NB
        win32file.WriteFile(pipe, some_data)       
        
    finally:
        win32file.CloseHandle(pipe)
       
#this applications client used to recieve and print messages
def pipe_c():
    end = False
    while not end:
        try:
            f = open(r'\\.\pipe\ThorImageConsoleTestPipe')

            #Read str length
            s = f.read(100)         # Read str
            f.seek(0)                               # Important!!!
            end = print_client_cmd(s)
            #put client enum handler here before printing-------------------------------------------------------------------------------------------------------------------------
            
            #print('Read:', s)
            time.sleep(2)
        except:
            time.sleep(.5)
        else:
            f.close()
    
    exit()

if __name__ == '__main__':
    xmlPath = ''
    remotePCHost = ''
    SaveName = "C:\\Temp\\exp01\\"
    

    print("Default experiment save path used: " + SaveName)
    c = Process(target=pipe_c)
    c.start()

    pipe_server()     #send connections establish
    time.sleep(.5)
    
    
    print("s - start acquisition.")
    print("l - load Experiment File.")
    print("c - change experiment path.")
    print("x - stop acquisition.")
    print("m - move stage.")
    print("r - request stage position.")
    print("Esc - end application.")
    mainLoop = True
    
    while mainLoop:
        
        key = get_key().lower()
        if key == 's':
            # Handle 's' key press
            st = "Remote~Local~StartAcquiring~" + SaveName 
            b = bytes(st, 'utf-16')

            pipe_out("Remote~Local~UpdateInformation~true/0")
            time.sleep(.02)
            pipe_out(st)

            print("Starting Experiment")

        elif key == 'l':
            # Handle 'l' key press
            sys.stdout.write("Enter the full path of the xml file to be loaded:\n-> ")
            sys.stdout.flush()
            xmlPath = input()
            pipe_out("Remote~Local~LoadExperimentFile~" + xmlPath)
            print("Experiment Path was successfully loaded from " + xmlPath)

        elif key == 'c':
            # Handle 'c' key press
            sys.stdout.write("Enter the new experiment path:\n-> ")
            sys.stdout.flush()
            SaveName = input() + "\\"
            print("Experiment Path was successfully loaded from " + SaveName)

        # elif key == 'h':
        #     # Handle 'h' key press
        #     sys.stdout.write("Enter the new Local Computer Name: ")
        #     sys.stdout.flush()
        #     remotePCHost = input()
        #     print("PC Host Name was successfully changed to " + remotePCHost)
            
        elif key == 'x':
            # Handle 'x' key press
            pipe_out("Remote~Local~StopAcquiring~0")
            print("Experiment Stopped")
        elif key == 'm':
            # Handle 'm' key press
            sys.stdout.write("Select the stage you want to move. \n Options are: X, Y, Z, or 2(for secondary z): ")
            sys.stdout.flush()

            stageInput = get_key().lower()
            sys.stdout.write("\n")
            sys.stdout.flush()
            if stageInput != 'x' and stageInput != 'y' and stageInput != 'z' and stageInput != '2':
                print('That stage does not exist') 
            else:
                print('Set the position to move to in micrometers: ')
                distance = input()
                if stageInput == 'x':
                    pipe_out("Remote~Local~MoveX~"+str(distance))
                    print('Stage X was successfully moved to ' + distance)
                elif stageInput == 'y':
                    pipe_out("Remote~Local~MoveY~"+str(distance))
                    print('Stage Y was successfully moved to ' + distance)
                elif stageInput == 'z':
                    pipe_out("Remote~Local~MoveZ~"+str(distance))
                    print('Stage Z (main stage) was successfully moved to ' + distance)
                elif stageInput == '2':
                    pipe_out("Remote~Local~MoveSecondaryZ~"+str(distance))
                    print('Stage Secondary Z was successfully moved to ' + distance)
            
        elif key == 'r':
            # Handle 'r' key press
            sys.stdout.write("Select the stage you want to request the position from in micrometers. \n Options are: X, Y, Z, or 2(for secondary z): ")
            sys.stdout.flush()

            stageInput = get_key().lower()
            sys.stdout.write("\n")
            sys.stdout.flush()
            if stageInput != 'x' and stageInput != 'y' and stageInput != 'z' and stageInput != '2':
                print('That stage does not exist') 
            else:
                if stageInput == 'x':
                    pipe_out("Remote~Local~ReportPositionX~0")
                elif stageInput == 'y':
                    pipe_out("Remote~Local~ReportPositionY~0")
                elif stageInput == 'z':
                    pipe_out("Remote~Local~ReportPositionZ~0")
                elif stageInput == '2':
                    pipe_out("Remote~Local~ReportPositionSecondaryZ~0")

        elif key == chr(27):  # chr(27) represents the Escape key
            # Handle Escape key press
            print("You pressed Escape. Exiting...")
            pipe_out("Remote~Local~TearDown~0")
            time.sleep(1)
            mainLoop = False
            break
        else:
            # Handle other keys
            print("Invalid key. Try again.")        
    
    c.terminate()
    pipe_out("Remote~Local~TearDown~0")

    

    
    