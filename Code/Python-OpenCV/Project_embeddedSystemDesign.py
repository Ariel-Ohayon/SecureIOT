import numpy as np
import cv2
import serial.tools.list_ports

def main():
    checkSerialPortConnection()
    ObjectRecognition()
    
    
    
def checkSerialPortConnection():
    # -- Initialize Serial Ports -- #
    ports = serial.tools.list_ports.comports()
    serialInst = serial.Serial()

    portlist = []
    for onePort in ports:
        portlist.append(str(onePort))
    if (portlist == []):
        print('COM PORTs not Found')
    #   return None
    else:
        for onePort in ports:
            print(str(onePort))
    # -- Serial Port Detection -- #
    
def ObjectRecognition():
    
    

    net = cv2.dnn.readNet('models/yolov3.weights','models/yolov3.cfg')
    classes = []
    with open('models/coco.names','r') as f:
        classes = f.read().splitlines()
    
    camera = cv2.VideoCapture(0)
    
    while True:
        
        _,frame = camera.read()
        frame = cv2.flip(frame,1)
        
        #img = cv2.imread('images/img.jpg')
        height, width, _ = frame.shape
    
    
        blob = cv2.dnn.blobFromImage(frame,1/255,(416,416),(0,0,0),swapRB = True,crop=False)
    
        # for b in blob:
            # for n,imgblob in enumerate(b):
                # cv2.imshow(str(n),imgblob)
        net.setInput(blob)
    
        output_layers_names = net.getUnconnectedOutLayersNames()
        layerOutputs = net.forward(output_layers_names)
    
        boxes = []
        confidences = []
        class_ids = []
    
        for output in layerOutputs:
            for detection in output:
                scores = detection[5:]
                class_id = np.argmax(scores)
                confidence = scores[class_id]
                if confidence>0.5:
                    center_x = int(detection[0]*width)
                    center_y = int(detection[1]*height)
                    w = int(detection[2]*width)
                    h = int(detection[3]*height)
                
                    x = int(center_x - w/2)
                    y = int(center_y - h/2)
                    boxes.append([x,y,w,h])
                    confidences.append(float(confidence))
                    class_ids.append(class_id)

        indexes = cv2.dnn.NMSBoxes(boxes,confidences,0.5,0.4)
    
        font = cv2.FONT_HERSHEY_PLAIN
        colors = np.random.uniform(0,255,size = (len(boxes),3))
    
        for i in indexes.flatten():
            x,y,w,h = boxes[i]
            label = str(classes[class_ids[i]])
            confidence = str(round(confidences[i],2))
            color = colors[i]
            cv2.rectangle(frame,(x,y),(x+w,y+h),color,2)
            cv2.putText(frame,label+" "+confidence,(x,y+20),font,2,(255,255,255),2)
        
        cv2.imshow('WebCam',frame)
        
        key = cv2.waitKey(1)
        if key == ord('q'):
            break
    camera.release()
    cv2.destroyAllWindows()
    

main()
