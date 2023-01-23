import numpy as np
import cv2

import serial.tools.list_ports

def main():
    Connection = serport()   # Initialize serial port for UART communication
    
    cam = cv2.VideoCapture(1)
    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
    eye  = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')
    
    while (True):
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
        
        if any(item in face for item in face):
            x = face[0][0]
            y = face[0][1]
            if ((x+(w/2)) < 240-10):
                Connection.write(b'1') # Send '1'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Right')
            elif ((x+(w/2)) > 240+10):
                Connection.write(b'0') # Send '0'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Left')
            if ((y+(h/2)) < 250-10):
                Connection.write(b'2') # Send '1'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Up')
            elif ((y+(h/2)) > 250+10):
                Connection.write(b'3') # Send '0'
                print('x: ',(x+(w/2)),'y: ',(y+(h/2)),' Move Down')
        
        cv2.imshow('WebCam Device',frm)
        
        #x=160
        
            
        if (cv2.waitKey(1) == ord('q')):
            break

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
