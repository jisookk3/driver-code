#name "Somfy-RTS Motor-v1905"
#brand "Somfy"
#type "RTS Motor"
#model "RTS Motor"
#version "1.0"
#author "Jisook"
#comments "RTS Motor(wireless) control - mylink"

/////////
/////////  Sending Json commands - Parsing data - control the RTS Motor

// You can get info below from the Integration Report via myLink - user should enter into Compass project
extern string Device_Auth; // "System_ID"
extern string Device_IP_Address;
extern string Device_Target_ID; 
extern string Device_Name; // ex. Roller Shade
extern int Device_Port; // 44100 for myLink

int KD_Active;
int KD_State;
string Debug;
string String_Buffer;



////////// Json commands to control RTS Motor
const string MOVE_UP = "mylink.move.up";
const string MOVE_DOWN = "mylink.move.down";
const string MOVE_STOP = "mylink.move.stop";
const string STATUS_PING = "mylink.status.ping";


// For now, just move up/down/stop the motor
int #Index_cmd;
string Json_cmd[3#Index_cmd];

const string CMD_MOVE_UP_EX = " { \"method\": \"mylink.move.up\", 
                              \"params\": { \"targetID\": \"CC107587.1\", \"auth\": \"kd1\" }, \"id\": 1 } ";



extern Start_Control();
extern Stop_Control();
extern Send_Command();
extern Control_Move_Up();
extern Control_Move_Down();
extern Control_Move_Stop();
extern Control_ping();
extern Get_response();




init() {
     KD_State = 0;
     Debug = "";


}

/*
wakeup() {
     Stop_Control();
     Start_Control();
}
*/
Start_Control() {
     KD_Active = 1;
     KD_State = 0;
}

Stop_Control() {
     KD_Active = 0;
     CloseControl();
}

main() {

     if(KD_Active == 0) return;

     Get_response();


}


Get_response() {
     string Response;
     Response = "null";
     String_Buffer = "";    
     
     while(Response != "") {
          Response = "";
          OpenControl(0);
          Delay(0.2);
          GetControlBefore(Response);
          LockStart();
          String_Buffer = String_Buffer + Response;
          LockStop();
     }
     CloseControl();

     Debug = Debug + String_Buffer + "\r\n";

     //temp_buffer = String_Buffer;
}


Send_Command(string command) {
     string cmd;
     cmd = command + "\r";
     OpenControl(0);
     SendControl(cmd);
     Delay(0.2);
     CloseControl();
}
////

/// { "method": "mylink.move.up", "params": { "targetID": "CC107587.1", "auth": "kd1" }, "id": 1 }

Control_Move_Up() {
     string cmd = "";
     sprintf(cmd, "{ \"method\": \"%s\", \"params\": { \"targetID\": \"%s\", \"auth\": \"%s\" }, \"id\": 1 }" ,
                    MOVE_UP, Device_Target_ID, Device_Auth);
     //cmd = cmd + "\r";
     Debug = Debug + cmd + "\r\n";
     Send_Command(cmd);
}


Control_Move_Down() {
     string cmd = "";
     sprintf(cmd, "{ \"method\": \"%s\", \"params\": { \"targetID\": \"%s\", \"auth\": \"%s\" }, \"id\": 1 }" ,
                    MOVE_DOWN, Device_Target_ID, Device_Auth);
          
     Debug = Debug + cmd + "\r\n";
     Send_Command(cmd);
}

Control_Move_Stop() {
     string cmd = "";
     sprintf(cmd, "{ \"method\": \"%s\", \"params\": { \"targetID\": \"%s\", \"auth\": \"%s\" }, \"id\": 1 }" ,
                    MOVE_STOP, Device_Target_ID, Device_Auth);
          
     Debug = Debug + cmd + "\r\n";
     Send_Command(cmd);
}

// {"method":"mylink.status.ping","params":{"targetID":"*","Auth":"kd1"},"id":1}
Control_ping() {
     string cmd = "";
     sprintf(cmd, "{ \"method\": \"%s\", \"params\": { \"targetID\": \"%s\", \"auth\": \"%s\" }, \"id\": 1 }" ,
                    STATUS_PING, Device_Target_ID, Device_Auth);
          
     Debug = Debug + cmd + "\r\n";
     Send_Command(cmd);

}



// GetJSON(<source>,<dest json>, <tail string>);





