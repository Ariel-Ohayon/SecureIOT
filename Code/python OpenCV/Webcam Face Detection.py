import numpy as np
import time
import cv2

import serial.tools.list_ports

def main():
    Mode = 0;
    Connection = serport()   # Initialize serial port for UART communication
    print("Wait for system to upload...")
    cam = cv2.VideoCapture(1)
    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
    
    while (Mode == 0):
        Mode = Connection.read() # Wait for send b'1' trough serial port
        
    Mode = 0;
    print("The system uploaded successfully.")
    while ((Mode != "1") and (Mode != "2")):
        print("Please choose a mode:")
        print("1. Automate Tracking.")
        print("2. Manually Control.")
        Mode = input()
        print(Mode)
        time.sleep(5)
        if (Mode == "1"):
            print("Enter to Automate Tracking mode.")
            Connection.write(b'1')
        elif (Mode == "2"):
            print("Enter to Manually Control mode.")
            Connection.write(b'2')
        else:
            print("Error.")
            print("Enter again mode of operation.")
    #while (True): // * Add this line after finish integration * \\
    while (Mode == "1"):
        ret,frm = cam.read()
        #frm = cv2.flip(frm,1)
        gray = cv2.cvtColor(frm,cv2.COLOR_BGR2GRAY)
        face = face_cascade.detectMultiScale(gray,1.3,5)
        if any(item in face for item in face):
            print('x: ',face[0][0], 'y: ',face[0][1])
        for (x,y,w,h) in face:
            cv2.rectangle(frm,(x,y),(x+w,y+h),(255,0,0),5)
            roi_gray = gray[y:y+h , x:x+w]
            roicolor = frm [y:y+h , x:x+w]
            flag = 0
        if any(item in face for item in face):
            x = face[0][0]
            y = face[0][1]
            print('Detect face.')
            Connection.write(b'4');    # Send '4' - for detection
            if ((x+(w/2)) < 240-50):
                flag = 1
                Connection.write(b'1') # Send '1'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Right')
                time.sleep(0.1);
            elif ((x+(w/2)) > 240+50):
                flag = 1
                Connection.write(b'0') # Send '0'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Left')
                time.sleep(0.1)
            if ((y+(h/2)) < 250-50):
                flag = 1
                Connection.write(b'2') # Send '2'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Up')
                time.sleep(0.1)
            elif ((y+(h/2)) > 250+50):
                flag = 1
                Connection.write(b'3') # Send '3'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Down')
                time.sleep(0.1)
            if (flag == 1):
                x = Connection.read()
                #print(x)
                if (x == b'1'):
                    print("Acknowledge !!!")
                elif (x == b'2'):
                    print("Change mode to Manual Control.")
                    Mode = "2";
                else:
                    print("Not Acknowledge !!!")
        else:
#            x = Connection.read();
            print('face not detect')
            Connection.write(b'4');
        
        cv2.imshow('WebCam Device',frm)
        if (cv2.waitKey(1) == ord('q')):
            cam.release()
            cv2.destroyAllWindows()
            Connection.close()
            return None
    
    flag = 0    
    while (Mode == "2"):
        if (flag == 0):
            print("You are in manual control.")
            print("Play with the joystick to control the camera")
            flag = 1
 
        ret,frm = cam.read()
        gray = cv2.cvtColor(frm,cv2.COLOR_BGR2GRAY)
        face = face_cascade.detectMultiScale(gray,1.3,5)
        if any(item in face for item in face):
            print('x: ',face[0][0], 'y: ',face[0][1])
        for (x,y,w,h) in face:
            cv2.rectangle(frm,(x,y),(x+w,y+h),(255,0,0),5)
            roi_gray = gray[y:y+h , x:x+w]
            roicolor = frm [y:y+h , x:x+w]
            
#        x = Connection.read()
#        if (x == b'1'):
            

        cv2.imshow('WebCam Device',frm)
        
        
        if (cv2.waitKey(1) == ord('q')):
            cam.release()
            cv2.destroyAllWindows()
            Connection.close()
            return None
    
    
def serport():
        # -- Initialize Serial Ports -- #
    ports = serial.tools.list_ports.comports()
    serialInst = serial.Serial()

    portlist = []
    for onePort in ports:
        portlist.append(str(onePort))
    if (portlist == []):
        print('COM PORTs not Found')
        return None
    else:
        print('List of the COM Ports:')
        for onePort in ports:
            print(str(onePort))
        # -- Serial Port Detection -- #
        
        
        # -- Connect to serial Port -- #
        COM = input('Enter The number of COM Port you want to connect: ')
        ConnPort = 'COM' + str(COM)
        print(f"connect to COM Port: {ConnPort}")
        Connection = serial.Serial(ConnPort,9600)
        Connection.baudrate = 9600
        Connection.parity = 'N'
        Connection.bytesize = 8
        Connection.stopbits = 1
        return Connection
    
main()
