#include "system_op.h"

String receivedMsg = "";  // Variable to store the complete message
String sendbackMsg = "";
uint8_t sendbackTimes = 0;
char cstr[256];

bool isBusy = false; // Flag for checking if the system is currently doing something else or not

void serial_sendback(){
    Serial.println(sendbackMsg);
    // Empty the sendback char string
    sendbackMsg = "";
}

#ifdef SYSOP_DEBUG
void compileACK(){
    sendbackMsg = "ACK-";
    sendbackMsg += receivedMsg;
    serial_sendback();
}
#endif

void process_string(){
    receivedMsg.trim();
    // Every command will return NUM_SAMPLES of samples.
    // Returning OBJECT TEMPERATURE results
    if (receivedMsg.equals("TEMP-GETOBJ")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif
        
        for (sendbackTimes = 0; sendbackTimes < NUM_SAMPLES; sendbackTimes++){
            updateTempData();
            sendbackMsg = "OBJ-";
            sendbackMsg += getObjTemp();
            serial_sendback();
            delay(1000);
        }
        return;
    }

    // Returning AMBIENT TEMPERATURE results
    if (receivedMsg.equals("TEMP-GETAMB")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif

        for (sendbackTimes = 0; sendbackTimes < NUM_SAMPLES; sendbackTimes++){
            updateTempData();
            sendbackMsg = "AMB-";
            sendbackMsg += getAmbTemp();
            serial_sendback();
            delay(1000);
        }
        return;
    }

    // Returning BPM and OXYGEN SATURATION results
    if (receivedMsg.equals("POX-GETDATA")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif

        sendbackTimes = 0;
        while(sendbackTimes < NUM_SAMPLES){
            updatePoxData();
            if(validBPM() && validSPO()){
                sendbackMsg = "POX-";
                sendbackMsg += getBPM();
                sendbackMsg += "-";
                sendbackMsg += getSpO2();
                serial_sendback();
                sendbackTimes++;
            }
        }
        return;
    }

    if (receivedMsg.equals("SCALE-GETWEIGHT")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif

        sendbackTimes = 0;
        while(sendbackTimes < NUM_SAMPLES){
            if(validSendWeight()){
                sendbackMsg = "WEIGHT-";
                sendbackMsg += getDefWeight();
                serial_sendback();
                sendbackTimes++;
            }else{
                sendbackMsg = "SCALE-ERROR-NODATA";
                serial_sendback();
                break;
            }
        }
        resetWeightSendFlag();
        return;
    }

    // SERVO SECTION
    if (receivedMsg.equals("SERVO-MOVEUP")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif
        servo_setMoveFlag(SERVO_MOVE_UP);
        return;
    }

    if (receivedMsg.equals("SERVO-MOVEDN")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif
        servo_setMoveFlag(SERVO_MOVE_DOWN);
        return;
    }

    if (receivedMsg.equals("SERVO-STOP")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif
        servo_setMoveFlag(SERVO_MOVE_STOP);
        return;
    }

    if (receivedMsg.equals("SERVO-MOVEDEFAULT")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif
        servo_setMoveFlag(SERVO_MOVE_DEFAULT);
        resetWeightSendFlag();
        return;
    }

    if (receivedMsg.equals("SERVO-GETANGLE")){
        #ifdef SYSOP_DEBUG
        compileACK();
        #endif

        sendbackTimes = 0;
        while(sendbackTimes < NUM_SAMPLES){
            sendbackMsg = "ANGLE-";
            sendbackMsg += servo_getPos();
            serial_sendback();
            sendbackTimes++;
        }
        return;
    }
    

    // NACK SENDBACK
    sendbackMsg = "NACK-SYNTAXERROR-";
    sendbackMsg += receivedMsg;
    serial_sendback();
}

void serial_receive(){
    while (Serial.available()) {
        char incomingChar = Serial.read();  // Read each character from the buffer
        if (incomingChar == '\n') {  // Check if the user pressed Enter (new line character)
            // Print the message
            isBusy = true;
            process_string();

            // Clear the message buffer for the next input
            isBusy = false;
            receivedMsg = "";
        } else if(incomingChar == '\b' && receivedMsg == ""){
            // There's nothing in the string but the user pressed backspace
            receivedMsg = "";
        } else {
            // Append the character to the message string
            receivedMsg += incomingChar;
        }
    }   
}

uint8_t system_busy(){
    return isBusy;
}